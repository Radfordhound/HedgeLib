#ifndef HL_CRI_ATOM_WAVE_BANK_H_INCLUDED
#define HL_CRI_ATOM_WAVE_BANK_H_INCLUDED

#include "../hl_internal.h"
#include "../common/io/hl_endian_readers.h"
#include "../common/io/hl_endian_writers.h"
#include <rad/rad_vector.h>
#include <rad/rad_span.h>
#include <rad/rad_stack_or_heap_array.h>

namespace rad
{
class stream;
}

namespace hl::cri_new::audio
{
class cue_sheet;
}

namespace hl::cri_new::atom
{
constexpr u32 wave_bank_signature = 0x32534641U; // AFS2

struct wave_bank_entry
{
    u16     id;
    u32     unalignedDataPos;
};

struct wave_bank_info
{
    u16     dataAlignment;
};

class wave_bank_reader
    : public little_endian_reader
{
    using entries_t_ = rad::stack_or_heap_array<wave_bank_entry, 32>;

    wave_bank_info  info_;
    entries_t_      entries_;

public:
    inline const wave_bank_info& info() const noexcept
    {
        return info_;
    }

    inline std::size_t waveform_count() const noexcept
    {
        // NOTE: This is safe because we ensure entries_.size()
        // is always at least 1 in the constructor.
        return entries_.size() - 1;
    }

    inline const entries_t_& entries() const noexcept
    {
        return entries_;
    }

    HL_API rad::vector<unsigned char> read_waveform_data_by_index(
        std::size_t index
    );

    HL_API rad::vector<unsigned char> read_waveform_data_by_id(
        u16 id
    );

    HL_API wave_bank_reader(rad::stream& stream);
};

class wave_bank_writer
{
    enum sequence_ : unsigned char
    {
        SEQ_NONE_,
        SEQ_HEADER_,
        SEQ_IDS_,
        SEQ_DATA_POSITIONS_,
        SEQ_DATA_,
    };

    little_endian_writer    writer_;
    unsigned long long      headerPos_;
    unsigned long long      curDataPositionPos_;
    unsigned long long      curDataEndPos_;
    sequence_               lastSeqStep_ = SEQ_NONE_;
    unsigned char           idSize_;
    unsigned char           dataPosSize_;
    unsigned long           waveformCount_;
    unsigned short          dataAlignment_;

    void fill_data_position_(unsigned long long unalignedDataPos);

public:
    HL_API void start(
        u16 dataAlignment = 32,
        u16 subkey = 0,
        u8 dataPosSize = 4,
        u8 idSize = 2
    );

    HL_API void write_id(u16 id);

    HL_API void write_data_positions();

    HL_API void write_data(rad::span<const unsigned char> rawData);

    HL_API void fill_data_position(
        unsigned long long unalignedDataPos,
        unsigned long long dataSize
    );

    HL_API void finish();

    inline wave_bank_writer(rad::stream& stream) noexcept
        : writer_(stream)
    {
    }
};
}

#endif
