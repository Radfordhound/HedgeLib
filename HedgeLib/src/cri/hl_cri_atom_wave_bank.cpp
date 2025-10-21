#include "hedgelib/cri/hl_cri_atom_wave_bank.h"
#include "hedgelib/cri/hl_cri_cue_sheet.h"
#include "hedgelib/common/io/hl_endian_writers.h"

namespace hl::cri_new::atom
{
rad::vector<unsigned char> wave_bank_reader::read_waveform_data_by_index(
    std::size_t index)
{
    if (index + 1 >= entries_.size())
    {
        throw std::out_of_range("The given index was out of range");
    }

    const auto& entry = entries_[index];
    const auto alignedDataPos = hl::align(
        entry.unalignedDataPos,
        info_.dataAlignment
    );

    stream().jump_to(alignedDataPos);

    const auto dataSize = (
        entries_[index + 1].unalignedDataPos -
        alignedDataPos
    );

    rad::vector<unsigned char> waveformData(rad::no_value_init, dataSize);
    stream().read_as(waveformData.data(), dataSize);

    return waveformData;
}

rad::vector<unsigned char> wave_bank_reader::read_waveform_data_by_id(
    u16 id)
{
    for (std::size_t i = 0; i < waveform_count(); ++i)
    {
        if (entries_[i].id == id)
        {
            return read_waveform_data_by_index(i);
        }
    }

    throw std::runtime_error("No waveform was found with the given id");
}

wave_bank_reader::wave_bank_reader(rad::stream& stream)
    : little_endian_reader(stream)
{
    // Read header.
    if (read_u32() != wave_bank_signature)
    {
        throw std::runtime_error("Unsupported AWB data format");
    }

    const auto version = read_u8();

    if (version != 2)
    {
        throw std::runtime_error("Unsupported AWB data version");
    }

    const auto waveformDataPosSize = read_u8();
    const auto waveformIDSize = read_u8();
    const auto unknown1 = read_u8();
    const auto waveformCount = read_u32();
    info_.dataAlignment = read_u16();
    const auto subkey = read_u16(); // TODO: Use this!!!

    // Read IDs.
    if (waveformIDSize != 2)
    {
        throw std::runtime_error("Unsupported waveform ID size");
    }

    entries_.assign(rad::no_value_init, waveformCount + 1);

    for (unsigned long i = 0; i < waveformCount; ++i)
    {
        entries_[i].id = read_u16();
    }

    entries_[waveformCount].id = UINT16_MAX;

    // Read unaligned data positions.
    switch (waveformDataPosSize)
    {
    case 2:
        for (unsigned long i = 0; i < waveformCount + 1; ++i)
        {
            entries_[i].unalignedDataPos = read_u16();
        }
        break;

    case 4:
        for (unsigned long i = 0; i < waveformCount + 1; ++i)
        {
            entries_[i].unalignedDataPos = read_u32();
        }
        break;

    default:
        throw std::runtime_error("Unsupported waveform data position size");
    }
}

void wave_bank_writer::fill_data_position_(unsigned long long unalignedDataPos)
{
    writer_.stream().jump_to(curDataPositionPos_);

    switch (dataPosSize_)
    {
    case 2:
        if (unalignedDataPos > UINT16_MAX)
        {
            throw std::overflow_error("Waveform data position is too large");
        }

        writer_.write_u16(static_cast<u16>(unalignedDataPos));
        curDataPositionPos_ += 2;
        break;

    case 4:
        if (unalignedDataPos > UINT32_MAX)
        {
            throw std::overflow_error("Waveform data position is too large");
        }

        writer_.write_u32(static_cast<u32>(unalignedDataPos));
        curDataPositionPos_ += 4;
        break;

    default:
        assert(false &&
            "Invalid waveform data position size; this should never happen!"
        );
        break;
    }
}

void wave_bank_writer::start(
    u16 dataAlignment,
    u16 subkey,
    u8 dataPosSize,
    u8 idSize)
{
    assert(lastSeqStep_ == SEQ_NONE_ &&
        "The writer functions are being called in an incorrect order"
    );

    if (idSize != 2)
    {
        throw std::runtime_error("Unsupported waveform ID size");
    }

    if (dataPosSize != 2 && dataPosSize != 4)
    {
        throw std::runtime_error("Unsupported waveform data position size");
    }

    headerPos_ = writer_.stream().tell();

    writer_.write_u32(wave_bank_signature); // signature
    writer_.write_u8(2); // version
    writer_.write_u8(dataPosSize); // waveformDataPosSize
    writer_.write_u8(idSize); // waveformIDSize
    writer_.write_u8(0); // unknown1
    writer_.write_u32(0); // waveformCount
    writer_.write_u16(dataAlignment); // waveformDataAlignment
    writer_.write_u16(subkey); // subkey

    idSize_ = idSize;
    dataPosSize_ = dataPosSize;
    waveformCount_ = 0;
    dataAlignment_ = dataAlignment;
    lastSeqStep_ = SEQ_HEADER_;
}

void wave_bank_writer::write_id(u16 id)
{
    assert(lastSeqStep_ == SEQ_HEADER_ || lastSeqStep_ == SEQ_IDS_ &&
        "The writer functions are being called in an incorrect order"
    );

    switch (idSize_)
    {
    case 2:
        writer_.write_u16(id);
        break;

    default:
        assert(false &&
            "Invalid waveform ID size; this should never happen!"
        );
        break;
    }

    ++waveformCount_;
    lastSeqStep_ = SEQ_IDS_;
}

void wave_bank_writer::write_data_positions()
{
    assert(lastSeqStep_ == SEQ_IDS_ ||
        (lastSeqStep_ == SEQ_HEADER_ && waveformCount_ == 0) &&
        "The writer functions are being called in an incorrect order"
    );

    curDataPositionPos_ = writer_.stream().tell();
    writer_.stream().write_nulls(dataPosSize_ * (waveformCount_ + 1));

    curDataEndPos_ = writer_.stream().tell();
    lastSeqStep_ = SEQ_DATA_POSITIONS_;
}

void wave_bank_writer::write_data(rad::span<const unsigned char> rawData)
{
    assert(lastSeqStep_ == SEQ_DATA_POSITIONS_ || lastSeqStep_ == SEQ_DATA_ &&
        "The writer functions are being called in an incorrect order"
    );

    assert(curDataPositionPos_ < // lastDataPositionPos:
        ((headerPos_ + 16) +
        (idSize_ * waveformCount_) +
        (dataPosSize_ * waveformCount_)) &&
        "write_data is being called too many times"
    );

    // Fill-in waveform unaligned data position.
    const auto unalignedDataPos = writer_.stream().tell();
    fill_data_position_(unalignedDataPos);

    // Align waveform data.
    writer_.stream().jump_to(unalignedDataPos);
    writer_.stream().pad(dataAlignment_);

    // Write waveform data.
    writer_.stream().write(rawData.data(), rawData.size());

    lastSeqStep_ = SEQ_DATA_;
}

void wave_bank_writer::fill_data_position(
    unsigned long long unalignedDataPos,
    unsigned long long dataSize)
{
    assert(lastSeqStep_ == SEQ_DATA_POSITIONS_ || lastSeqStep_ == SEQ_DATA_ &&
        "The writer functions are being called in an incorrect order"
    );

    assert(curDataPositionPos_ < // lastDataPositionPos:
        ((headerPos_ + 16) +
        (idSize_ * waveformCount_) +
        (dataPosSize_ * waveformCount_)) &&
        "fill_data_position is being called too many times"
    );

    // Fill-in waveform unaligned data position.
    fill_data_position_(unalignedDataPos);

    // Update data end position.
    const auto alignedDataPos = align(unalignedDataPos, dataAlignment_);
    const auto dataEndPos = alignedDataPos + dataSize;

    if (dataEndPos > curDataEndPos_)
    {
        curDataEndPos_ = dataEndPos;
    }

    lastSeqStep_ = SEQ_DATA_;
}

void wave_bank_writer::finish()
{
    assert(lastSeqStep_ == SEQ_DATA_ ||
        (lastSeqStep_ == SEQ_DATA_POSITIONS_ && waveformCount_ == 0) &&
        "The writer functions are being called in an incorrect order"
    );

    // Fill-in end position.
    const auto endPos = writer_.stream().tell();
    fill_data_position_(std::max<>(endPos, curDataEndPos_));

    // Fill-in waveform count.
    writer_.stream().jump_to(headerPos_ + 8);
    writer_.write_u32(waveformCount_);

    // Jump back to end.
    writer_.stream().jump_to(endPos);

    lastSeqStep_ = SEQ_NONE_;
}
}
