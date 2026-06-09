#include <rad/rad_stream.h>

#include "hedgelib/io/hl_endian_readers.h"
#include "hedgelib/cri/hl_cri_atom_wave_bank.h"

namespace hl::cri::atom
{
rad::vector<unsigned char> wave_bank_deserializer::read_waveform_data_by_index(
    std::size_t index,
    rad::allocator& allocator)
{
    if (index >= entries_.size() - 1)
    {
        throw std::out_of_range("The given index was out of range");
    }

    const auto& entry = entries_[index];
    const auto alignedDataPos = hl::align(
        entry.unalignedDataPos,
        info_.dataAlignment
    );

    stream_->jump_to(alignedDataPos);

    const auto dataSize = (
        entries_[index + 1].unalignedDataPos -
        alignedDataPos
    );

    rad::vector<unsigned char> waveformData(rad::no_value_init, allocator, dataSize);
    stream_->read_as(waveformData.data(), dataSize);

    return waveformData;
}

rad::vector<unsigned char> wave_bank_deserializer::read_waveform_data_by_id(
    u16 id,
    rad::allocator& allocator)
{
    for (std::size_t i = 0; i < waveform_count(); ++i)
    {
        if (entries_[i].id == id)
        {
            return read_waveform_data_by_index(i, allocator);
        }
    }

    throw std::runtime_error("No waveform was found with the given id");
}

wave_bank_deserializer::wave_bank_deserializer(rad::stream& stream)
    : stream_(&stream)
{
    // Read header.
    io::little_endian_reader reader(stream);
    if (reader.read_u32() != wave_bank_signature)
    {
        throw std::runtime_error("Unsupported AWB data format");
    }

    const auto version = reader.read_u8();

    if (version > 2)
    {
        throw std::runtime_error("Unsupported AWB data version");
    }

    info_.dataPosSize = reader.read_u8();
    info_.idAlignment = reader.read_u8();
    const auto unknown1 = reader.read_u8();
    const auto waveformCount = reader.read_u32();
    info_.dataAlignment = reader.read_u16();

    // NOTE: In version 1, this is a reserved, unused field.
    // However, there's no harm in reading it and passing its
    // value to the subkey field anyway.
    info_.subkey = reader.read_u16();

    // Read IDs.
    if constexpr (SIZE_MAX <= UINT32_MAX)
    {
        // NOTE: We're not checking the waveform count, but the
        // waveform *entry* count, which is the waveform count + 1.
        if (waveformCount >= SIZE_MAX)
        {
            throw std::overflow_error("AWB waveform entry count exceeds size_t range");
        }
    }

    entries_.assign(rad::no_value_init, static_cast<std::size_t>(waveformCount) + 1);

    if (info_.idAlignment > 2)
    {
        for (u32 i = 0; i < waveformCount; ++i)
        {
            // TODO: Is this correct?
            stream.align(info_.idAlignment);
            entries_[i].id = reader.read_u16();
        }
    }
    else
    {
        for (u32 i = 0; i < waveformCount; ++i)
        {
            entries_[i].id = reader.read_u16();
        }
    }

    entries_[waveformCount].id = UINT16_MAX;

    // Read unaligned data positions.
    switch (info_.dataPosSize)
    {
    case 2:
        for (u32 i = 0; i < waveformCount + 1; ++i)
        {
            entries_[i].unalignedDataPos = reader.read_u16();
        }
        break;

    case 4:
        for (u32 i = 0; i < waveformCount + 1; ++i)
        {
            entries_[i].unalignedDataPos = reader.read_u32();
        }
        break;

    default:
        throw std::runtime_error("Unsupported AWB waveform data position size");
    }
}

void wave_bank_serializer::write_data_position_(unsigned long long unalignedDataPos)
{
    switch (waveBankInfo_.dataPosSize)
    {
    case 2:
        if (unalignedDataPos > UINT16_MAX)
        {
            throw std::overflow_error("AWB waveform data position is too large");
        }

        writer_.write_u16(static_cast<u16>(unalignedDataPos));
        break;

    case 4:
        if (unalignedDataPos > UINT32_MAX)
        {
            throw std::overflow_error("AWB waveform data position is too large");
        }

        writer_.write_u32(static_cast<u32>(unalignedDataPos));
        break;

    default:
        assert(false &&
            "Invalid AWB waveform data position size; this should never happen!"
        );
        break;
    }
}

wave_bank_serializer::waveform_resolver::waveform_resolver(
    wave_bank_serializer& serializer,
    unsigned long long firstOffPos) noexcept
    : serializer_(&serializer)
    , curOffPos_(firstOffPos)
{
}

void wave_bank_serializer::waveform_resolver::start()
{
    assert(serializer_->lastSeqStep_ == SEQ_DATA_POSITIONS_ &&
        "This resolver cannot be used after finish() "
        "has been called on its associated serializer"
    );

    // TODO: Somehow validate that start() was not called
    // again without a finish() call inbetween.

    // Fill-in waveform unaligned data position.
    auto writer = serializer_->writer_;
    const auto unalignedDataPos = writer.stream().tell();

    writer.stream().jump_to(curOffPos_);
    serializer_->write_data_position_(unalignedDataPos);

    // Update state.
    curOffPos_ += serializer_->waveBankInfo_.dataPosSize;

    // Align waveform data.
    writer.stream().jump_to(unalignedDataPos);
    writer.stream().pad(serializer_->waveBankInfo_.dataAlignment);
}

void wave_bank_serializer::waveform_resolver::finish()
{
    assert(serializer_->lastSeqStep_ == SEQ_DATA_POSITIONS_ &&
        "This resolver cannot be used after finish() "
        "has been called on its associated serializer"
    );

    // TODO: Somehow validate that start() was called first?

    // Update data section end position.
    const auto dataEndPos = serializer_->writer_.stream().tell();

    assert(dataEndPos >= serializer_->dataSectionEndPos_ &&
        "finish() must be called with the stream position set to "
        "the end of the associated waveform"
    );
    
    serializer_->dataSectionEndPos_ = dataEndPos;
}

wave_bank_serializer::toc_waveform_resolver::toc_waveform_resolver(
    wave_bank_serializer& serializer,
    unsigned long long firstOffPos) noexcept
    : serializer_(&serializer)
    , curOffPos_(firstOffPos)
{
}

void wave_bank_serializer::toc_waveform_resolver::next(u32 promisedWaveformSize)
{
    assert(serializer_->lastSeqStep_ == SEQ_DATA_POSITIONS_ &&
        "This resolver cannot be used after finish() "
        "has been called on its associated serializer"
    );

    // Fill-in waveform unaligned data position.
    auto writer = serializer_->writer_;

    writer.stream().jump_to(curOffPos_);
    serializer_->write_data_position_(serializer_->dataSectionEndPos_);

    // Update state.
    curOffPos_ += serializer_->waveBankInfo_.dataPosSize;
    serializer_->dataSectionEndPos_ = (
        align(
            serializer_->dataSectionEndPos_,
            serializer_->waveBankInfo_.dataAlignment) +
        promisedWaveformSize
    );
}

void wave_bank_serializer::start(wave_bank_info waveBankInfo)
{
    assert(lastSeqStep_ == SEQ_NONE_ &&
        "start() must not be called again until after "
        "a matching call to finish()"
    );

    if (waveBankInfo.version > 2)
    {
        throw std::runtime_error("Unsupported AWB version");
    }

    if (waveBankInfo.dataPosSize != 2 && waveBankInfo.dataPosSize != 4)
    {
        throw std::runtime_error("Unsupported AWB waveform data position size");
    }

    headerPos_ = writer_.stream().tell();

    writer_.write_u32(wave_bank_signature); // signature
    writer_.write_u8(waveBankInfo.version); // version
    writer_.write_u8(waveBankInfo.dataPosSize); // dataPosSize
    writer_.write_u8(waveBankInfo.idAlignment); // idAlignment
    writer_.write_u8(0); // unknown1
    writer_.write_u32(0); // waveformCount
    writer_.write_u16(waveBankInfo.dataAlignment); // dataAlignment
    writer_.write_u16((waveBankInfo.version > 1) ? waveBankInfo.subkey : 0); // subkey

    waveBankInfo_ = waveBankInfo;
    waveformCount_ = 0;
    lastSeqStep_ = SEQ_HEADER_;
}

void wave_bank_serializer::write_id(u16 id)
{
    assert(lastSeqStep_ > SEQ_NONE_ &&
        "start() must be called before write_id()"
    );

    assert(lastSeqStep_ < SEQ_DATA_POSITIONS_ &&
        "write_id() should not be called again after "
        "starting the data section or calling finish()"
    );

    if (waveBankInfo_.idAlignment > 2)
    {
        writer_.stream().pad(waveBankInfo_.idAlignment);
    }

    writer_.write_u16(id);

    ++waveformCount_;
    lastSeqStep_ = SEQ_IDS_;
}

auto wave_bank_serializer::begin_data_section() -> waveform_resolver
{
    assert(lastSeqStep_ > SEQ_NONE_ &&
        "start() must be called before starting the data section"
    );

    assert(lastSeqStep_ < SEQ_DATA_POSITIONS_ &&
        "The data section must not be started "
        "more than once per call to start()"
    );

    // Write placeholder offsets.
    const auto curOffPos = writer_.stream().tell();
    writer_.stream().write_nulls(
        (static_cast<std::size_t>(waveformCount_) + 1) *
        waveBankInfo_.dataPosSize
    );

    // Return offset resolver.
    dataSectionBeginPos_ = dataSectionEndPos_ = writer_.stream().tell();
    lastSeqStep_ = SEQ_DATA_POSITIONS_;

    return waveform_resolver(*this, curOffPos);
}

auto wave_bank_serializer::begin_toc_data_section() -> toc_waveform_resolver
{
    assert(lastSeqStep_ > SEQ_NONE_ &&
        "start() must be called before starting the data section"
    );

    assert(lastSeqStep_ < SEQ_DATA_POSITIONS_ &&
        "The data section must not be started "
        "more than once per call to start()"
    );

    // Write placeholder offsets.
    const auto curOffPos = writer_.stream().tell();
    writer_.stream().write_nulls(
        (static_cast<std::size_t>(waveformCount_) + 1) *
        waveBankInfo_.dataPosSize
    );

    // Return offset resolver.
    dataSectionBeginPos_ = dataSectionEndPos_ = writer_.stream().tell();
    lastSeqStep_ = SEQ_DATA_POSITIONS_;

    return toc_waveform_resolver(*this, curOffPos);
}

unsigned long long wave_bank_serializer::finish()
{
    assert(lastSeqStep_ > SEQ_NONE_ &&
        "start() must be called before finish()"
    );

    assert(lastSeqStep_ == SEQ_DATA_POSITIONS_ &&
        "The data section must be started before calling finish()"
    );

    // Fill-in data section end position.
    const auto endPos = writer_.stream().tell();
    writer_.stream().jump_to(dataSectionBeginPos_ - waveBankInfo_.dataPosSize);
    write_data_position_(dataSectionEndPos_);

    // Fill-in waveform count.
    writer_.stream().jump_to(headerPos_ + 8);
    writer_.write_u32(waveformCount_);

    // Jump back to end.
    writer_.stream().jump_to(endPos);

    lastSeqStep_ = SEQ_NONE_;
    return dataSectionBeginPos_;
}
}
