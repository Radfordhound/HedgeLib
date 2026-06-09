#ifndef HL_CRI_AUDIO_CUE_SHEET_H_INCLUDED
#define HL_CRI_AUDIO_CUE_SHEET_H_INCLUDED

#include <utility>
#include <array>

#include <rad/rad_vector.h>
#include <rad/rad_string.h>
#include <rad/rad_span.h>

#include "../hl_internal.h"
#include "hl_cri_utf.h"

namespace rad
{
class stream;
}

namespace hl::cri::audio
{
struct cue_sheet;

struct graph_point
{
    /// @brief The input value, normalized to 0 - 10000 inclusive.
    u16 in;

    /// @brief The output value, normalized to 0 - 10000 inclusive.
    u16 out;

    constexpr graph_point(u16 in, u16 out) noexcept
        : in(in)
        , out(out)
    {
    }
};

enum class graph_type : u8
{
    volume = 0,
    pitch = 1,
    bandpass_cutoff_low = 3,
    bandpass_cutoff_high = 4,
    bus_send_0 = 12, // TODO: Is this correct?
    bus_send_1 = 13,
    unknown23 = 23,
    voice_priority = 24,
    unknown26 = 26,
};

struct graph
{
    graph_type                  type;
    float                       inputMax;
    float                       inputMin;
    float                       outputMax;
    float                       outputMin;
    rad::vector<graph_point>    points;

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
        float inputMax,
        float inputMin,
        float outputMax,
        float outputMin,
        rad::vector<graph_point> points) noexcept
        : type(type)
        , inputMax(inputMax)
        , inputMin(inputMin)
        , outputMax(outputMax)
        , outputMin(outputMin)
        , points(std::move(points))
    {
    }
};

enum class aisac_type : u8
{
    simple = 0,
    // TODO: It looks like there's a random type.
};

struct aisac
{
    rad::string         name;
    rad::string         controlName;
    aisac_type          type;
    u8                  randomRange;
    rad::vector<graph>  graphs;

    aisac(
        rad::string name,
        rad::string controlName,
        aisac_type type,
        u8 randomRange = 0) noexcept
        : name(std::move(name))
        , controlName(std::move(controlName))
        , type(type)
        , randomRange(randomRange)
    {
    }

    aisac(
        rad::string name,
        rad::string controlName,
        aisac_type type,
        u8 randomRange,
        rad::vector<graph> graphs) noexcept
        : name(std::move(name))
        , controlName(std::move(controlName))
        , type(type)
        , randomRange(randomRange)
        , graphs(std::move(graphs))
    {
    }
};

enum class sound_element_format : u8
{
    aax = 0,
};

struct sound_element
{
    rad::string                 name;
    sound_element_format        format;
    u8                          channelCount;
    u32                         sampleRate;
    u32                         sampleCount;

    /// @brief Embedded audio data. If empty, the sound element is streamed.
    rad::vector<unsigned char>  embeddedData;

    HL_API u32 compute_play_duration() const noexcept;

    sound_element(
        rad::string name,
        sound_element_format format,
        u8 channelCount,
        u32 sampleRate,
        u32 sampleCount) noexcept
        : name(std::move(name))
        , format(format)
        , channelCount(channelCount)
        , sampleRate(sampleRate)
        , sampleCount(sampleCount)
    {
    }

    sound_element(
        rad::string name,
        sound_element_format format,
        u8 channelCount,
        u32 sampleRate,
        u32 sampleCount,
        rad::vector<unsigned char> embeddedData) noexcept
        : name(std::move(name))
        , format(format)
        , channelCount(channelCount)
        , sampleRate(sampleRate)
        , sampleCount(sampleCount)
        , embeddedData(std::move(embeddedData))
    {
    }
};

enum class synth_link_type : u8
{
    sound_element = 0,
    synth = 1,
};

enum class synth_complex_type : u8
{
    polyphonic = 0,
    random_no_repeat = 1,
    sequential = 2,
    random = 3,
    sequential_no_loop = 6,
};

// TODO: What filter is this?
struct synth_filter2_info
{
    u8 type = 0; // TODO: What is this? Always seems to be set to 0.
    u16 cofLo = 0; // TODO: What is this? Coefficient Low O?
    u16 cofLg = 1000; // TODO: What is this? Coefficient Low G?
    u16 cofHo = 1000; // TODO: What is this? Coefficient High O? Also default to 0 for recursive synths
    u16 cofHg = 1000; // TODO: What is this? Coefficient High G?
};

// TODO: What filter is this?
struct synth_filter1_info
{
    u8 type = 0; // TODO: What is this? Always seems to be set to 0.
    u16 cofO = 0; // TODO: What is this?
    u16 cofG = 0; // TODO: What is this?
    u16 resoO = 0; // TODO: What is this?
    u16 resoG = 0; // TODO: What is this?
};

struct synth_pan_3d_info
{
    s16 volume = 1000; // TODO: Default to 0 for recursive synths
    s16 vg = 1000; // TODO: What is this?
    s16 ao = 1000; // TODO: What is this?
    s16 ag = 1000; // TODO: What is this? Angle?
    s16 ido = 1000; // TODO: What is this? Also default to 0 for recursive synths
    s16 idg = 1000; // TODO: What is this? Also default to 0 for recursive synths
};

struct synth_voice_limit_info
{
    rad::optional_string groupName;
    u8 type = 0; // TODO: Make this an enum class. ACB has "Last Voice Priority" and "First Voice Priority"
    u8 priority = 0;
    u16 phTime = 0; // TODO: What is this?
    s8 pcDlt = 0; // TODO: What is this? Pc Delay Time?
};

// TODO: What filter is this?
struct synth_filter0_info
{
    u8 type = 0; // TODO: What is this? Always seems to be set to 0.
    u16 coefficient1 = 0;
    u16 coefficient2 = 0;
    u16 resolve = 0;
    u8 rolloff = 0;
};

struct synth_envelope_info
{
    u16 delay = 0;
    u16 attack = 0;
    u16 hold = 0;
    u16 decay = 0;
    u16 release = 0;
    u16 sustain = 1000;
};

struct synth
{
    rad::string                         name;
    synth_link_type                     linkType;
    synth_complex_type                  complexType;
    rad::vector<rad::string>            linkNames;
    rad::vector<rad::string>            aisacNames;
    s16                                 volume = 1000;
    s16                                 pitch = 0;
    u32                                 delayTime = 0;
    u8                                  sControl = 0;
    synth_envelope_info                 envelopeInfo;
    synth_filter0_info                  filter0Info;
    rad::optional_string                dryOutName;
    rad::optional_string                mtxrtr;
    std::array<u16, 8>                  dry = {}; // TODO: What is this?
    rad::optional_string                wetOutName;
    std::array<u16, 8>                  wet = {}; // TODO: What is this?
    std::array<rad::optional_string, 8> wetCnct; // TODO: What is this? Cn control?
    synth_voice_limit_info              voiceLimitInfo;
    synth_pan_3d_info                   pan3dInfo;
    std::array<u8, 8>                   dryG = { 255, 255, 255, 255, 255, 255, 255, 255 }; // TODO: What is this? Gain?
    std::array<u8, 8>                   wetG = { 255, 255, 255, 255, 255, 255, 255, 255 }; // TODO: What is this? Gain?
    synth_filter1_info                  filter1Info;
    synth_filter2_info                  filter2Info;
    u8                                  probability = 100; // TODO: Default to 0 for recursive synths.
    u8                                  numLimitChildren = 0;
    bool                                repeat = false; // TODO: Default to true for recursive synths.
    u32                                 comboTime = 0;
    u8                                  comboLoopBack = 0; // TODO: Is this a boolean?

    synth(
        rad::string name,
        synth_link_type linkType,
        synth_complex_type complexType,
        rad::allocator& allocator = rad::default_allocator) noexcept
        : name(std::move(name))
        , linkType(linkType)
        , complexType(complexType)
        , linkNames(allocator)
        , aisacNames(allocator)
        , dryOutName(allocator)
        , mtxrtr(allocator)
        , wetOutName(allocator)
        , wetCnct{
            rad::optional_string{allocator},
            rad::optional_string{allocator},
            rad::optional_string{allocator},
            rad::optional_string{allocator},
            rad::optional_string{allocator},
            rad::optional_string{allocator},
            rad::optional_string{allocator},
            rad::optional_string{allocator},
        }
        , voiceLimitInfo{{rad::optional_string{allocator}}}
    {
    }
};

enum cue_flags : u8
{
    CUE_FLAGS_NONE = 0,
    CUE_FLAGS_DOES_LOOP = 1,
};

struct cue
{
    u32                     id;
    u8                      flags = CUE_FLAGS_NONE;
    rad::optional_string    name; // TODO: Is this actually optional ?
    rad::string             synthName;
    rad::optional_string    userData;

    cue(u32 id,
        rad::string synthName,
        u8 flags = CUE_FLAGS_NONE,
        rad::optional_string name = nullptr,
        rad::optional_string userData = nullptr) noexcept
        : id(id)
        , flags(flags)
        , name(std::move(name))
        , synthName(std::move(synthName))
        , userData(std::move(userData))
    {
    }
};

// TODO: Move to another header
namespace detail_
{
    template<typename Iterator>
    Iterator find_named(std::string_view name, Iterator begin, Iterator end)
    {
        while (begin != end)
        {
            if (begin->name == name)
            {
                return begin;
            }

            ++begin;
        }

        return begin;
    }
}

struct cue_sheet
{
    rad::vector<aisac>          aisacs;
    rad::vector<sound_element>  soundElements;
    rad::vector<synth>          synths;
    rad::vector<cue>            cues;

    using aisac_const_iterator          = rad::vector<aisac>::const_iterator;
    using aisac_iterator                = rad::vector<aisac>::iterator;
    using sound_element_const_iterator  = rad::vector<sound_element>::const_iterator;
    using sound_element_iterator        = rad::vector<sound_element>::iterator;
    using synth_const_iterator          = rad::vector<synth>::const_iterator;
    using synth_iterator                = rad::vector<synth>::iterator;
    using cue_const_iterator            = rad::vector<cue>::const_iterator;
    using cue_iterator                  = rad::vector<cue>::iterator;

    aisac_const_iterator find_aisac(std::string_view name) const
    {
        return detail_::find_named(name, aisacs.begin(), aisacs.end());
    }

    inline aisac_iterator find_aisac(std::string_view name)
    {
        return const_cast<aisac_iterator>(
            const_cast<const cue_sheet*>(this)->find_aisac(name)
        );
    }

    sound_element_const_iterator find_sound_element(std::string_view name) const
    {
        return detail_::find_named(name, soundElements.begin(), soundElements.end());
    }

    inline sound_element_iterator find_sound_element(std::string_view name)
    {
        return const_cast<sound_element_iterator>(
            const_cast<const cue_sheet*>(this)->find_sound_element(name)
        );
    }

    synth_const_iterator find_synth(std::string_view name) const
    {
        return detail_::find_named(name, synths.begin(), synths.end());
    }

    inline synth_iterator find_synth(std::string_view name)
    {
        return const_cast<synth_iterator>(
            const_cast<const cue_sheet*>(this)->find_synth(name)
        );
    }

    HL_API void clear() noexcept;

    HL_API void read_inner(
        rad::stream& stream,
        rad::allocator& tmpAllocator = rad::default_allocator
    );

    HL_API void read(
        rad::stream& stream,
        rad::allocator& tmpAllocator = rad::default_allocator
    );

    explicit cue_sheet(
        rad::allocator& allocator = rad::default_allocator) noexcept
        : aisacs(allocator)
        , soundElements(allocator)
        , synths(allocator)
        , cues(allocator)
    {
    }

    explicit cue_sheet(
        rad::stream& stream,
        rad::allocator& tmpAllocator = rad::default_allocator,
        rad::allocator& allocator = rad::default_allocator)
        : cue_sheet(allocator)
    {
        read(stream, tmpAllocator);
    }
};
}

#endif
