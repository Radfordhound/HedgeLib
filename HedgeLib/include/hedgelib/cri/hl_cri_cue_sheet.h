#ifndef HL_CRI_CUE_SHEET_H_INCLUDED
#define HL_CRI_CUE_SHEET_H_INCLUDED

#include <ankerl/unordered_dense.h>

#include "../hl_internal.h"
#include "hl_cri_utf.h"
#include <rad/rad_vector.h>
#include <rad/rad_string.h>
#include <rad/rad_span.h>
#include <utility>

namespace rad
{
class stream;
}

namespace hl::cri_new::audio
{
class cue_sheet;

class revision_info
{
    utf::column_info_group  columns_;
    u16                     totalColumnCount_;

public:
    constexpr const utf::column_info_group& columns() const noexcept
    {
        return columns_;
    }

    constexpr u16 total_column_count() const noexcept
    {
        return totalColumnCount_;
    }

    constexpr revision_info(
        rad::span<const utf::column_info_range> columnRanges) noexcept
        : columns_{columnRanges}
        , totalColumnCount_(columns_.get_total_count())
    {
    }
};

HL_API extern const utf::column_info aisac_columns[];

HL_API extern const rad::span<const revision_info> aisac_revisions;

HL_API extern const utf::column_info sound_element_columns[];

HL_API extern const rad::span<const revision_info> sound_element_revisions;

HL_API extern const utf::column_info synth_columns[];

HL_API extern const rad::span<const revision_info> synth_revisions;

HL_API extern const utf::column_info cue_columns[];

HL_API extern const rad::span<const revision_info> cue_revisions;

HL_API const revision_info* get_revision_info(
    const utf::table_deserializer& td,
    rad::span<const revision_info> revisionInfos
) noexcept;

struct graph_point
{
    /// @brief The input value, normalized to 0 - 10000 inclusive.
    u16     in;

    /// @brief The output value, normalized to 0 - 10000 inclusive.
    u16     out;

    constexpr graph_point(u16 in, u16 out) noexcept
        : in(in)
        , out(out)
    {
    }
};

enum class graph_type : u8
{
    volume = 0,
    bandpass_cutoff_low = 3,
    bandpass_cutoff_high = 4,
    bus_send_0 = 12, // TODO: Is this correct?
    bus_send_1 = 13,
};

class graph
{
public:
    graph_type                  type;
    rad::vector<graph_point>    points;
    float                       inputMax = 1.0f;
    float                       inputMin = 0.0f;
    float                       outputMax = 1.0f;
    float                       outputMin = 0.0f;

    inline graph(graph_type type,
        float inputMax = 1.0f,
        float inputMin = 0.0f,
        float outputMax = 1.0f,
        float outputMin = 0.0f) noexcept
        : type(type)
        , inputMax(inputMax)
        , inputMin(inputMin)
        , outputMax(outputMax)
        , outputMin(outputMin)
    {
    }

    inline graph(graph_type type,
        rad::vector<graph_point> points,
        float inputMax = 1.0f,
        float inputMin = 0.0f,
        float outputMax = 1.0f,
        float outputMin = 0.0f) noexcept
        : type(type)
        , points(std::move(points))
        , inputMax(inputMax)
        , inputMin(inputMin)
        , outputMax(outputMax)
        , outputMin(outputMin)
    {
    }
};

enum class aisac_type : u8
{
    simple = 0,
    // TODO: It looks like there's a random type.
};

class aisac
{
public:
    rad::string         controlName;
    aisac_type          type;
    u8                  randomRange;
    rad::vector<graph>  graphs;
};

enum sound_element_format : u8
{
    SOUND_ELEMENT_FORMAT_AAX = 0,
};

HL_API sound_element_format parse_sound_element_format(u8 val);

class sound_element
{
public:
    rad::vector<unsigned char>  embeddedData;
    sound_element_format        format;
    unsigned char               channelCount;
    unsigned long               sampleRate;
    unsigned long               sampleCount;

    // TODO

    HL_API u32 compute_play_duration() const noexcept;
};

enum synth_link_type : u8
{
    /// @brief The indices in `nodeLinkIndices` refer to sound element nodes.
    SYNTH_LINK_TYPE_SOUND_ELEMENT = 0,

    /// @brief The indices in `nodeLinkIndices` refer to synth nodes.
    SYNTH_LINK_TYPE_SYNTH = 1,
};

HL_API synth_link_type parse_synth_link_type(u8 val);

enum synth_complex_type : u8
{
    SYNTH_COMPLEX_TYPE_POLYPHONIC = 0,
    SYNTH_COMPLEX_TYPE_RANDOM_NO_REPEAT = 1,
    SYNTH_COMPLEX_TYPE_SEQUENTIAL = 2,
    SYNTH_COMPLEX_TYPE_RANDOM = 3,
    SYNTH_COMPLEX_TYPE_SEQUENTIAL_NO_LOOP = 6,
};

HL_API synth_complex_type parse_synth_complex_type(u8 val);

class synth
{
public:
    synth_link_type             linkType;
    synth_complex_type          complexType;
    rad::vector<rad::string>    nodeLinkNames;
    rad::vector<rad::string>    aisacNames;
    s16                         volume;
    s16                         pitch;
    s16                         delayTime;
    u8                          sControl;
    u16                         egDelay;
    u16                         egAttack;
    u16                         egHold;
    u16                         egDecay;
    u16                         egRelease;
    u16                         egSuspend;
    // TODO

    HL_API u32 compute_play_duration(const cue_sheet& cueSheet) const;
};

enum cue_flags : u8
{
    CUE_FLAGS_NONE = 0,
    CUE_FLAGS_DOES_LOOP = 1,
};

HL_API void validate_cue_flags(u8 val);

class cue
{
public:
    u32                     id = 0;
    u8                      flags = CUE_FLAGS_NONE;
    rad::optional_string    name;
    rad::string             synthName;
    rad::optional_string    userData;

    HL_API const synth& get_synth(const cue_sheet& cueSheet) const;

    HL_API u32 compute_play_duration(const cue_sheet& cueSheet) const;

    inline cue() noexcept = default;

    HL_API cue(
        u32 id,
        rad::string synthName,
        u8 flags = CUE_FLAGS_NONE,
        rad::optional_string name = nullptr,
        rad::optional_string userData = nullptr
    ) noexcept;
};

namespace detail_
{
    template<typename T>
    using map = ankerl::unordered_dense::map<
        rad::string,
        T,
        ankerl::unordered_dense::hash<rad::string>,
        std::equal_to<rad::string>,
        std::allocator<std::pair<rad::string, T>> // TODO: Replace with rad::allocator
    >;
}

using aisac_map = detail_::map<aisac>;
using aisac_const_iterator = aisac_map::const_iterator;
using aisac_iterator = aisac_map::iterator;

using sound_element_map = detail_::map<sound_element>;
using sound_element_const_iterator = sound_element_map::const_iterator;
using sound_element_iterator = sound_element_map::iterator;

using synth_map = detail_::map<synth>;
using synth_const_iterator = synth_map::const_iterator;
using synth_iterator = synth_map::iterator;

class cue_sheet
{
    void read_aisac_table_(rad::stream& stream);

    void read_sound_element_table_(rad::stream& stream);

    void read_synth_table_(rad::stream& stream);

    void read_cue_table_(rad::stream& stream);

public:
    aisac_map           aisacs;
    sound_element_map   soundElements;
    synth_map           synths;
    rad::vector<cue>    cues;

    inline rad::allocator& allocator() const noexcept
    {
        // TODO: Remove this function
        return cues.allocator();
    }

    HL_API void clear() noexcept;

    HL_API void read_inner(rad::stream& stream);

    HL_API void read(rad::stream& stream);

    cue_sheet() noexcept = default;

    HL_API explicit cue_sheet(rad::stream& stream);
};
} // hl::cri

#endif
