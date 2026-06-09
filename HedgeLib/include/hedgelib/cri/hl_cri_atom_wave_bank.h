#ifndef HL_CRI_ATOM_WAVE_BANK_H_INCLUDED
#define HL_CRI_ATOM_WAVE_BANK_H_INCLUDED

#include <rad/rad_vector.h>
#include <rad/rad_span.h>
#include <rad/rad_stack_or_heap_array.h>

#include "../hl_internal.h"
#include "../io/hl_endian_writers.h"

namespace rad
{
class stream;
}

namespace hl::cri::atom
{
constexpr u32 wave_bank_signature = 0x32534641U; // AFS2

struct raw_wave_bank_entry
{
    u16     id;
    u32     unalignedDataPos;
};

struct wave_bank_info
{
    /// @brief Version number.
    ///
    /// - v1: Initial version.
    /// - v2: Introduced around 2018. Adds subkey field, which
    /// was previously an unused, reserved field.
    u8                      version = 2;
    u8                      dataPosSize = 4;
    u16                     idAlignment = 2;
    u16                     dataAlignment = 32;
    u16                     subkey = 0;
};

class wave_bank_deserializer
{
    using entries_t_ = rad::stack_or_heap_array<raw_wave_bank_entry, 32>;

    rad::stream*            stream_;
    wave_bank_info          info_;
    entries_t_              entries_;

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
        std::size_t index,
        rad::allocator& allocator = rad::default_allocator
    );

    HL_API rad::vector<unsigned char> read_waveform_data_by_id(
        u16 id,
        rad::allocator& allocator = rad::default_allocator
    );

    HL_API wave_bank_deserializer(rad::stream& stream);
};

class wave_bank_serializer
{
    enum sequence_ : unsigned char
    {
        SEQ_NONE_,
        SEQ_HEADER_,
        SEQ_IDS_,
        SEQ_DATA_POSITIONS_,
    };

    io::little_endian_writer    writer_;
    unsigned long long          headerPos_ = 0;
    unsigned long long          dataSectionBeginPos_ = 0;
    unsigned long long          dataSectionEndPos_ = 0;
    wave_bank_info              waveBankInfo_;
    u32                         waveformCount_ = 0;
    sequence_                   lastSeqStep_ = SEQ_NONE_;

    void write_data_position_(unsigned long long unalignedDataPos);

public:
    class waveform_resolver
    {
        friend wave_bank_serializer;

        wave_bank_serializer*   serializer_;
        unsigned long long      curOffPos_;

        waveform_resolver(
            wave_bank_serializer& serializer,
            unsigned long long firstOffPos
        ) noexcept;

    public:
        inline rad::stream& stream() const noexcept
        {
            return serializer_->writer_.stream();
        }

        HL_API void start();

        HL_API void finish();

        waveform_resolver& operator=(const waveform_resolver&) = delete;

        waveform_resolver& operator=(waveform_resolver&&) noexcept = default;

        waveform_resolver(const waveform_resolver&) = delete;

        waveform_resolver(waveform_resolver&&) noexcept = default;
    };

    class toc_waveform_resolver
    {
        friend wave_bank_serializer;

        wave_bank_serializer*   serializer_;
        unsigned long long      curOffPos_;

        toc_waveform_resolver(
            wave_bank_serializer& serializer,
            unsigned long long firstOffPos
        ) noexcept;

    public:
        HL_API void next(u32 promisedWaveformSize);

        toc_waveform_resolver& operator=(const toc_waveform_resolver&) = delete;

        toc_waveform_resolver& operator=(toc_waveform_resolver&&) noexcept = default;

        toc_waveform_resolver(const toc_waveform_resolver&) = delete;

        toc_waveform_resolver(toc_waveform_resolver&&) noexcept = default;
    };

    HL_API void start(wave_bank_info waveBankInfo = {});

    HL_API void write_id(u16 id);

    HL_API waveform_resolver begin_data_section();

    HL_API toc_waveform_resolver begin_toc_data_section();

    HL_API unsigned long long finish();

    wave_bank_serializer& operator=(const wave_bank_serializer&) = delete;

    wave_bank_serializer& operator=(wave_bank_serializer&&) noexcept = default;

    wave_bank_serializer(rad::stream& stream) noexcept
        : writer_(stream)
    {
    }

    wave_bank_serializer(const wave_bank_serializer&) = delete;

    wave_bank_serializer(wave_bank_serializer&&) noexcept = default;
};
}

#endif
