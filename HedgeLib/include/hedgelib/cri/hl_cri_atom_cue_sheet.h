#ifndef HL_CRI_ATOM_CUE_SHEET_H_INCLUDED
#define HL_CRI_ATOM_CUE_SHEET_H_INCLUDED

#include <optional>

#include <rad/rad_span.h>
#include <rad/rad_string.h>
#include <rad/rad_vector.h>
#include <rad/rad_stack_or_heap_array.h>

#include "../hl_internal.h"
#include "../hl_guid.h"
#include "hl_cri_utf.h"
#include "hl_cri_atom.h"

namespace rad
{
class stream;
}

namespace hl::cri::atom
{
struct aisac_control
{
    rad::string     name;
    u16             id;

    inline aisac_control(rad::string name, u16 id) noexcept
        : name(std::move(name))
        , id(id)
    {
    }
};

enum class graph_type : u16
{
    volume = 1,
    pitch = 2,
    bandpass_cutoff_high = 3,
    bandpass_cutoff_low = 4,
    biquad_cutoff = 5,
    biquad_q = 6,
    bus_send_0 = 7,
    bus_send_1 = 8,
    bus_send_2 = 9,
    bus_send_3 = 10,
    bus_send_4 = 11,
    bus_send_5 = 12,
    bus_send_6 = 13,
    bus_send_7 = 14,
    pan3d_angle = 15,
    pan3d_volume = 16,
    pan3d_internal_distance = 17,
    aisac_control_1 = 21,
    aisac_control_2 = 22,
    aisac_control_3 = 23,
    aisac_control_4 = 24,
    aisac_control_5 = 25,
    aisac_control_6 = 26,
    aisac_control_7 = 27,
    aisac_control_8 = 28,
    aisac_control_9 = 29,
    aisac_control_10 = 30,
    aisac_control_11 = 31,
    aisac_control_12 = 32,
    aisac_control_13 = 33,
    aisac_control_14 = 34,
    aisac_control_15 = 35,
    voice_priority = 36,
    predelay_time = 37,
    biquad_gain = 38,
    pan3d_center = 39,
    pan3d_lfe = 40,
    eg_attack_time = 41,
    eg_release_time = 42,
    playback_rate = 43,
    pan3d_spread = 52,
};

struct graph
{
    graph_type              type;
    rad::vector<float>      controls;
    rad::vector<u16>        destinations;
    rad::vector<u16>        curves;

    graph(
        graph_type type,
        rad::allocator& allocator = rad::default_allocator) noexcept
        : type(type)
        , controls(allocator)
        , destinations(allocator)
        , curves(allocator)
    {
    }
};

// TODO: Merge this with waveform_loop_type ?
enum class auto_modulation_type : u8
{
    one_shot = 1,
    loop = 2,
};

enum class trigger_type : u8
{
    playback = 0,
    key = 1,
};

struct auto_modulation
{
    auto_modulation_type    type = auto_modulation_type::loop;
    trigger_type            triggerType = trigger_type::playback;

    /// @brief Time in milliseconds.
    // TODO: Perhaps duration would be a better name?
    u32                     time;

    /// @brief Used if triggerType == key.
    u32                     key = UINT32_MAX;

    constexpr auto_modulation(
        auto_modulation_type type,
        trigger_type triggerType,
        u32 time,
        u32 key = UINT32_MAX) noexcept
        : type(type)
        , triggerType(triggerType)
        , time(time)
        , key(key)
    {
    }
};

enum class aisac_type : u8
{
    simple = 0,
    automation = 1, // TODO: Is this a good name for this?
};

struct aisac
{
    /// TODO: What is this value for? It seems to never be set to anything but 0.
    s16                     id = 0;
    aisac_type              type;

    // TODO: This seems to be 1000 + ID if type == simple, 
    // 2000 + something if type == automation,
    // 3000 + something if auto modulation is used
    u16                     controlId;

    /// @brief Used if type == random. The control value will be adjusted
    /// by a random number within a range of this size, centered around
    /// the control value.
    ///
    /// Example: If the control value is 0.4, and the randomRange is 0.2,
    /// the control value will end up being a random number between
    /// 0.3 and 0.5, inclusive.
    ///
    /// To get the min valid random value: `(controlValue - randomRange / 2.0f)`
    /// To get the max valid random value: `(controlValue + randomRange / 2.0f)`
    float                   randomRange = 0.0f;

    u16                     autoModulationIndex = UINT16_MAX;

    /// @brief The indices of the graphs which comprise this AISAC.
    rad::vector<u16>        graphIndices;

    /// @brief If specified, represents the default value of the associated
    /// AISAC Control to be used when the game does not specify any value.
    std::optional<float>    defaultControl;

    // TODO: GraphBitFlag
    // TODO: GraphTypeIndexes

    aisac(
        aisac_type type,
        u16 controlId,
        rad::allocator& allocator = rad::default_allocator) noexcept
        : type(type)
        , controlId(controlId)
        , graphIndices(allocator)
    {
    }
};

enum class waveform_encode_type : u8
{
    /// @brief Cri ADX codec, supported by all platforms.
    /// @details Used in Sonic Lost World for Wii U/PC for bgm.
    adx = 0,

    /// @brief Cri HCA codec, supported by all platforms.
    /// @details Used in most games which use ADX2.
    hca = 2,

    //hw1 = 4, ?

    /// @brief Cri HCA-MX codec, supported by all platforms.
    hca_mx = 6,

    //hw2 = 8, ?
    // maybe ATRAC3 ?

    /// @brief Platform-specific CWAV codec used by 3DS.
    /// @details HW1 for 3DS? Used in Sonic Lost World for 3DS.
    cwav = 9,

    /// @brief Platform-specific ATRAC9 codec used by PS Vita, PS4, and PS5.
    /// @details HW2 for PS Vita? Used in Persona 4 Golden for PS Vita.
    atrac9 = 11,
};

enum class waveform_stream_type : u8
{
    /// @brief Stream the entry with the id specified by `memoryAwbId`
    /// from the `embeddedAwbData` in the cue sheet. Best used for
    /// sound effects, or other shorter waveforms which need to play
    /// frequently or immediately.
    memory = 0,
    /// @brief Stream the entry with the id specified by `streamAwbId`
    /// from the streaming file (usually an external .awb file, rarely
    /// also an external .cpk file) set by the game. Best used for music,
    /// or other longer waveforms.
    stream = 1,
    /// @brief This is intended to be used with two copies of the audio
    /// data - a smaller, truncated one in memory, and the full data in
    /// an external file. It begins streaming immediately from the entry
    /// with the id specified by `memoryAwbId`, presumably while the
    /// first batch of data is loaded from the streaming file. Then, it
    /// switches over to streaming the entry with the id specified by
    /// `streamAwbId` from the streaming file set by the game.
    stream_no_latency = 2,
};

enum waveform_loop_type : u8
{
    one_shot = 1,
    loop = 2,
};

struct waveform
{
    u16                         memoryAwbId;
    u16                         streamAwbId;
    waveform_encode_type        encodeType;
    waveform_stream_type        streamType;
    waveform_loop_type          loopType;
    u8                          channelCount;
    u32                         sampleRate;
    u32                         sampleCount;
    // TODO: ExtensionData
    u16                         streamAwbPort;
    // TODO: LipMorthIndex
    // TODO: ChConfig
    // TODO: HrtfType

    constexpr waveform(
        u16 memoryAwbId,
        u16 streamAwbId,
        waveform_encode_type encodeType,
        waveform_stream_type streamType,
        waveform_loop_type loopType,
        u8 channelCount,
        u32 sampleRate,
        u32 sampleCount,
        u16 streamAwbPort) noexcept
        : memoryAwbId(memoryAwbId)
        , streamAwbId(streamAwbId)
        , encodeType(encodeType)
        , streamType(streamType)
        , loopType(loopType)
        , channelCount(channelCount)
        , sampleRate(sampleRate)
        , sampleCount(sampleCount)
        , streamAwbPort(streamAwbPort)
    {
    }
};

struct track
{
    u16                     eventIndex = UINT16_MAX;
    u16                     commandIndex = UINT16_MAX;
    rad::vector<u16>        localAisacIndices;
    u16                     globalAisacStartIndex = UINT16_MAX;
    u16                     globalAisacCount = 0;
    // TODO: ParameterPallet
    // TODO: TargetType
    // TODO: TargetName
    // TODO: TargetId
    // TODO: TargetAcbName
    // TODO: Scope
    // TODO: TargetTrackNo
    // TODO: BeatSyncLabel
    // TODO: TargetIdentificationCondition

    inline track(rad::allocator& allocator = rad::default_allocator) noexcept
        : localAisacIndices(allocator)
    {
    }
};

// TODO: Should we combine sequence_type and synth_type ? Did I validate that they are actually the same?
enum class sequence_type : u8
{
    polyphonic = 0,
    sequential = 1,
    shuffle = 2,
    random = 3,
    random_no_repeat = 4,
    switch_game_variable = 5,
    combo_sequential = 6,
    switch_selector = 7,
    track_transition_by_selector = 8,
};

struct sequence
{
    u16                         playbackRatio = 100;
    rad::vector<u16>            trackIndices;
    u16                         commandIndex = UINT16_MAX;
    rad::vector<u16>            localAisacIndices;
    u16                         globalAisacStartIndex = UINT16_MAX;
    u16                         globalAisacCount = 0;
    // TODO: ParameterPallet
    // TODO: ActionTrackStartIndex
    // TODO: NumActionTracks
    rad::vector<u16>            trackValues;
    sequence_type               type = sequence_type::polyphonic;
    // TODO: NumPlaybackTrackNoHistories
    // TODO: InstPluginTrackStartIndex
    // TODO: NumInstPluginTracks
    // TODO: MIDITrackStartIndex
    // TODO: NumMIDITracks
    // TODO: WatchActionStartIndex
    // TODO: NumWatchAction
    // TODO: StopActionStartIndex
    // TODO: NumStopAction

    sequence(
        sequence_type type,
        rad::allocator& allocator = rad::default_allocator) noexcept
        : trackIndices(allocator)
        , localAisacIndices(allocator)
        , trackValues(allocator)
        , type(type)
    {
    }
};

enum class synth_type : u8
{
    polyphonic = 0,
    sequential = 1,
    shuffle = 2,
    random = 3,
    random_no_repeat = 4,
    switch_game_variable = 5,
    combo_sequential = 6,
    switch_selector = 7,
    track_transition_by_selector = 8,
};

struct synth
{
    synth_type                      type;
    rad::string                     voiceLimitGroupName;
    u16                             commandIndex = UINT16_MAX;
    rad::vector<ref_item>           refItems;
    rad::vector<u16>                localAisacIndices;
    u16                             globalAisacStartIndex = UINT16_MAX;
    u16                             globalAisacCount = 0;
    rad::vector<u16>                trackValues;
    // TODO: ParameterPallet
    // TODO: ActionTrackStartIndex
    // TODO: NumActionTracks

    synth(
        synth_type type,
        rad::allocator& allocator = rad::default_allocator) noexcept
        : type(type)
        , voiceLimitGroupName(allocator)
        , refItems(allocator)
        , localAisacIndices(allocator)
        , trackValues(allocator)
    {
    }
};

struct cue
{
    u32                             id;

    /// @brief The item which the cue will play when the cue is triggered.
    ///
    /// NOTE: For unknown reasons, the official Cri Atom libraries used by
    /// games seem to not handle the case very well where a cue references
    /// a synth which directly references a waveform. If you do this, you
    /// may encounter buggy behavior in-game, or even crashes! Instead, if
    /// a cue references a synth, that synth should **always** reference another
    /// synth. No such problems have yet been discovered with cues referencing
    /// anything else.
    ref_item                        refItem;

    rad::optional_string            name;
    rad::string                     userData;
    
    /// @brief Bit array, where each bit represents whether the
    /// AISAC Control at the corresponding index is used by the
    /// cue or not.
    rad::stack_or_heap_array<u8, 8> aisacControlMap;

    /// @brief How many milliseconds this cue should play for,
    /// or UINT32_MAX to loop indefinitely.
    u32                             playDuration = UINT32_MAX;
    bool                            headerVisibility = true;
    u16                             relatedWaveformCount = 0;

    cue(u32 id,
        ref_item refItem,
        rad::optional_string name = nullptr,
        rad::allocator& allocator = rad::default_allocator) noexcept
        : id(id)
        , refItem(refItem)
        , name(std::move(name))
        , userData(allocator)
        // TODO: Pass allocator into aisacControlMap
    {
    }
};

enum class config_ref_item_type : u8
{
    category = 3,
    aisac = 4,
    aisac_control = 5,
    voice_limit_group = 6,
    selector_label = 7,
    dsp_bus = 9,
};

struct config_ref_item
{
    config_ref_item_type        type;
    rad::string                 name;
    /// @brief Unused for most types. For selector labels, the name of the associated selector.
    rad::string                 name2;
    unsigned long               id;

    config_ref_item(
        config_ref_item_type type,
        rad::string name,
        rad::string name2 = {},
        unsigned long id = UINT32_MAX) noexcept
        : type(type)
        , name(std::move(name))
        , name2(std::move(name2))
        , id(id)
    {
    }
};

namespace detail_
{
    struct write_params
    {
        rad::stream*            stream;
        rad::allocator*         allocator;
        packed_version          version;
        utf::encoding_type      encoding;
        bool                    useGlobalCmdTable;
        unsigned short          globalCmdTableIndex = 0;

        //struct
        //{
            ////u16                 synth = 0;
            //u16                 track = 0;
            //u16                 sequence = 0;
        //}
        //globalAisacStartIndices;

        inline unsigned short get_command_index(
            unsigned short commandIndex) noexcept
        {
            // TODO:
            return commandIndex;
            //return (!useGlobalCmdTable || commandIndex == UINT16_MAX) ?
                //commandIndex : globalCmdTableIndex++;
        }
    };

    //void validate_reference_type(u8 type);
}

struct wave_bank_hash
{
    rad::string     name;
    md5_hash        md5Hash;

    wave_bank_hash(
        rad::string name,
        md5_hash md5Hash) noexcept
        : name(std::move(name))
        , md5Hash{md5Hash}
    {
    }
};

struct cue_sheet
{
    static constexpr packed_version latest_supported_version = packed_version(1, 42, 01);

    struct serialize_info
    {
        packed_version  version;
        encoding_type   encoding;

        // NOTE: This constructor only exists to work around
        // what seems to be a Clang (also gcc?) compiler bug?
        serialize_info(
            packed_version version = latest_supported_version,
            encoding_type encoding = encoding_type::utf8) noexcept
            : version(version)
            , encoding(encoding)
        {
        }
    };

    u32                                         fileIdentifier = 0;
    // TODO: Type
    // TODO: Target
    /// @brief MD5 hash of the complete ACF (Atom ConFiguration) file used by this cue sheet.
    md5_hash                                    acfMd5Hash = {};
    // TODO: CategoryExtension
    rad::vector<cue>                            cues;
    rad::vector<waveform>                       waveforms;
    rad::vector<aisac>                          aisacs;
    rad::vector<graph>                          graphs;
    rad::vector<rad::string>                    globalAisacNames;
    // TODO: AISACNameTable
    rad::vector<synth>                          synths;
    rad::vector<command_table>                  sequenceCommands;
    rad::vector<track>                          tracks;
    rad::vector<sequence>                       sequences;
    rad::vector<aisac_control>                  aisacControls;
    rad::vector<auto_modulation>                autoModulations;
    // TODO: StreamAwbTocWorkOld ??
    rad::vector<unsigned char>                  embeddedAwbData;
    // TODO: CueLimitWorkTable
    // TODO: NumCueLimitListWorks
    // TODO: NumCueLimitNodeWorks
    guid                                        id = guid::zero();
    /// @brief MD5 hash of each complete AWB (Atom Wave Bank) file used for streaming.
    rad::vector<wave_bank_hash>                 streamAwbHashes;
    // TODO: StreamAwbTocWork_Old ??
    float                                       volume = 1.0f;
    rad::vector<rad::string>                    stringValues;
    // TODO: OutsideLinkTable
    // TODO: BlockSequenceTable
    // TODO: BlockTable
    rad::string                                 name;
    // TODO: EventTable
    // TODO: ActionTrackTable
    rad::vector<config_ref_item>                acfRefItems;
    // TODO: WaveformExtensionDataTable
    // TODO: BeatSyncInfoTable
    // TODO: CuePriorityType
    // TODO: NumCueLimit
    rad::vector<command_table>                  trackCommands;
    rad::vector<command_table>                  synthCommands;
    rad::vector<command_table>                  trackEventCommands;
    // TODO: SeqParameterPalletTable
    // TODO: TrackParameterPalletTable
    // TODO: SynthParameterPalletTable
    // TODO: SoundGeneratorTable
    // TODO: InstrumentPluginTrackTable
    // TODO: InstrumentPluginParameterTable
    // TODO: LipsMorphTable
    // TODO: ProjectKey
    // TODO: SoundInstruments
    // TODO: SoundProgramBankKey
    // TODO: MIDITrackTable
    // TODO: SoundProgramBankCommandTable
    // TODO: ParameterAction
    // TODO: ParameterActionCondition
    // TODO: StopAction
    /// @brief One copy of each TOC (Table Of Contents) section from each
    /// AWB (Atom Wave Bank) file this cue sheet uses for streaming.
    rad::vector<rad::vector<unsigned char>> streamAwbTocData;

    HL_API bool has_any_command_tables() const noexcept;

    HL_API u16 get_related_waveform_count(const synth& synth) const;

    HL_API u16 get_related_waveform_count(const command_table& cmdTable) const;

    HL_API u16 get_related_waveform_count(const track& track) const;

    HL_API u16 get_related_waveform_count(const sequence& sequence) const;

    inline u16 get_related_waveform_count(const cue& cue) const
    {
        return get_related_waveform_count(cue.refItem);
    }

    HL_API u16 get_related_waveform_count(ref_item refItem) const;

    HL_API void clear() noexcept;

    HL_API serialize_info read_inner(
        rad::stream& stream,
        rad::allocator& tmpAllocator = rad::default_allocator
    );

    HL_API serialize_info read(
        rad::stream& stream,
        rad::allocator& tmpAllocator = rad::default_allocator
    );

    HL_API void write(
        rad::stream& stream,
        const serialize_info& serializeInfo = {},
        rad::allocator& tmpAllocator = rad::default_allocator
    ) const;

    HL_API explicit cue_sheet(
        rad::string name,
        guid id = guid::random(),
        rad::allocator& allocator = rad::default_allocator
    ) noexcept;

    HL_API explicit cue_sheet(
        rad::stream& stream,
        rad::allocator& tmpAllocator = rad::default_allocator,
        rad::allocator& allocator = rad::default_allocator
    );
};
}

#endif
