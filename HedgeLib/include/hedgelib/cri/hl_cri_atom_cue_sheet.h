#ifndef HL_CRI_ATOM_CUE_SHEET_H_INCLUDED
#define HL_CRI_ATOM_CUE_SHEET_H_INCLUDED

#include "../hl_internal.h"
#include "hl_cri_utf.h"
#include <rad/rad_string.h>
#include <rad/rad_vector.h>
#include <rad/rad_span.h>
#include <rad/rad_endian.h>
#include <rad/rad_stack_or_heap_array.h>
#include <optional>

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
class cue_sheet;

// TODO: Move stuff not specific to cue sheets into another header: hl_cri_atom.h
// This stuff is used by other atom formats too such as acf.

class packed_version
{
    u32 value_ = 0;

    static constexpr u8 decimal_to_hex_lookalike_(u8 val) noexcept
    {
        const u8 onesPlace = (val % 10);
        const u8 tensPlace = (val / 10);

        return (tensPlace * 16) + onesPlace;
    }

    static constexpr u32 create_packed_version_(u8 major, u8 minor, u8 rev) noexcept
    {
        return (
            (static_cast<u32>(decimal_to_hex_lookalike_(major)) << 24) |
            (static_cast<u32>(decimal_to_hex_lookalike_(minor)) << 16) |
            (static_cast<u32>(decimal_to_hex_lookalike_(rev)) << 8)
        );
    }

    static constexpr u8 hex_lookalike_to_decimal_(u8 val) noexcept
    {
        const u8 onesPlace = (val % 16);
        const u8 tensPlace = (val / 16);

        return (tensPlace * 10) + onesPlace;
    }

public:
    constexpr u32 value() const noexcept
    {
        return value_;
    }

    constexpr u8 get_major() const noexcept
    {
        const auto val = static_cast<u8>(
            static_cast<u32>(value_ & 0xFF000000U) >> 24
        );

        return hex_lookalike_to_decimal_(val);
    }

    constexpr u8 get_minor() const noexcept
    {
        const auto val = static_cast<u8>(
            static_cast<u32>(value_ & 0xFF0000U) >> 16
        );

        return hex_lookalike_to_decimal_(val);
    }

    constexpr u8 get_revision() const noexcept
    {
        const auto val = static_cast<u8>(
            static_cast<u32>(value_ & 0xFF00U) >> 8
        );

        return hex_lookalike_to_decimal_(val);
    }

    constexpr operator u32() const noexcept
    {
        return value_;
    }

    constexpr packed_version() noexcept = default;

    constexpr packed_version(u32 value) noexcept
        : value_(value)
    {
    }

    constexpr packed_version(u8 major, u8 minor, u8 rev) noexcept
        : value_(create_packed_version_(major, minor, rev))
    {
    }
};

struct revision_info
{
    packed_version version;
    utf::column_info_group columns;

    constexpr revision_info(
        packed_version version,
        rad::span<const utf::column_info_range> columnRanges) noexcept
        : version(version)
        , columns{columnRanges}
    {
    }

    inline constexpr u16 get_total_count() const noexcept
    {
        return columns.get_total_count();
    }
};

HL_API const revision_info* get_revision_info(
    rad::span<const revision_info> revisionInfos,
    packed_version version
) noexcept;

enum class reference_type : u8
{
    waveform = 1,
    synth = 2,
    sequence = 3,
    block_sequence = 8,
    none = 255,
};

class reference_item
{
public:
    reference_type      type = reference_type::none;
    u16                 index = 0;

    HL_API u16 compute_related_waveform_count(const cue_sheet& cueSheet) const;

    constexpr reference_item() noexcept = default;

    constexpr reference_item(reference_type type, u16 index) noexcept
        : type(type)
        , index(index)
    {
    }
};

enum class command_type : u16
{
    /// @brief Does nothing.
    /// @details no_op()
    no_op = 0x0000,

    /// @brief Sets the pitch to the given level.
    /// @details set_pitch(s16 pitch)
    /// @param pitch A value between [-1200 - 1200] which represents the new pitch.
    set_pitch = 0x0005,

    /// @brief Sets the category to use.
    /// @details set_category(u32[] categoryID)
    ///
    /// @param categoryID The ID of the category.
    set_category = 0x0041,

    /// @brief Sets the volume to the given level.
    /// @details set_volume(u16 volume)
    /// @param volume A value between [0 - 500] which represents the new volume level.
    set_volume = 0x0057,

    /// @brief Specifies that we want to send audio to
    /// the given bus with the given volume level.
    /// @details set_bus_send(string busSendName, u16 volume)
    /// @param busSendName The name of the bus to send audio to.
    /// @param volume A value between [0 - 10000] which represents the new volume level.
    set_bus_send = 0x006F,

    /// @brief Jumps back to the beginning of the given loop.
    /// @details seq_loop_end(u16 loopIndex, u32 loopDurationMilliseconds)
    /// @param loopIndex The index of the loop. Multiple loops can be used.
    /// @param loopDurationMilliseconds The duration of one play of the given loop, in milliseconds.
    seq_loop_end = 0x04B0,

    /// @brief Sets the current play position as the beginning of the given loop.
    /// @details seq_loop_start(u16 loopIndex, u16 loopRepeatCount)
    /// @param loopIndex The index of the loop. Multiple loops can be used.
    /// @param loopRepeatCount How many times this loop should repeat itself. 0xFFFF == infinite loop.
    seq_loop_start = 0x04B1,

    /// @brief Plays the given item.
    /// @details play(u16 refType, u16 refIndex)
    /// @param refType A `reference_type` value representing what type of item is being referenced.
    /// @param refIndex The index to the item being referenced.
    play = 0x07D0,

    /// @brief Waits for the given amount of time before executing the next command.
    /// @details wait(u32 numMilliseconds)
    /// @param numMilliseconds How many milliseconds to wait for.
    wait = 0x07D1,
};

class command
{
    command_type        type_;
    unsigned char       argCount_;

public:
    inline command_type type() const noexcept
    {
        return type_;
    }

    inline unsigned char argument_count() const noexcept
    {
        return argCount_;
    }

    constexpr command(command_type type, unsigned char argCount) noexcept
        : type_(type)
        , argCount_(argCount)
    {
    }
};

class command_table
{
public:
    rad::vector<unsigned char>  rawData;

    class command_reference
    {
        const unsigned char*    ptr_;
    
    public:
        inline command_type type() const noexcept
        {
            return static_cast<command_type>(
                rad::endian::big_to_native(*reinterpret_cast<const u16*>(ptr_))
            );
        }

        inline unsigned char argument_count() const noexcept
        {
            return *(ptr_ + sizeof(u16));
        }

        inline const void* arguments_big_endian() const noexcept
        {
            return (ptr_ + sizeof(u16) + 1);
        }

        inline explicit command_reference(const unsigned char* ptr) noexcept
            : ptr_(ptr)
        {
        }
    };

    class const_iterator
    {
        const unsigned char*    ptr_ = nullptr;

    public:
        using iterator_category     = std::forward_iterator_tag;
        using value_type            = const command_reference;
        using difference_type       = std::ptrdiff_t;
        using pointer               = ptr_proxy<const command_reference>;
        using reference             = command_reference;

        inline const unsigned char* as_raw_data_iterator() const noexcept
        {
            return ptr_;
        }

        inline reference operator*() const noexcept
        {
            return command_reference(ptr_);
        }

        inline pointer operator->() const noexcept
        {
            return command_reference(ptr_);
        }

        HL_API const_iterator& operator++() noexcept;

        HL_API const_iterator operator++(int) noexcept;

        inline friend bool operator==(
            const const_iterator& a,
            const const_iterator& b) noexcept
        {
            return (a.ptr_ == b.ptr_);
        }

        inline friend bool operator!=(
            const const_iterator& a,
            const const_iterator& b) noexcept
        {
            return (a.ptr_ != b.ptr_);
        }

        constexpr const_iterator() noexcept = default;

        inline explicit const_iterator(const unsigned char* rawDataIt) noexcept
            : ptr_(rawDataIt)
        {
        }
    };

    using iterator = const_iterator; // TODO

    inline const_iterator cbegin() const noexcept
    {
        return const_iterator(rawData.cbegin());
    }

    inline const_iterator begin() const noexcept
    {
        return const_iterator(rawData.cbegin());
    }

    inline const_iterator cend() const noexcept
    {
        return const_iterator(rawData.cend());
    }

    inline const_iterator end() const noexcept
    {
        return const_iterator(rawData.cend());
    }

    template<typename... Args>
    iterator insert(const_iterator pos, command_type type, Args... args)
    {
        // TODO: Remove this function and manually implement every insert instead.

        const auto argsCount = sizeof...(Args);
        const auto argsSize = (sizeof(Args) + ... + 0);

        static_assert(argsCount <= 255,
            "Commands cannot have more than 255 arguments"
        );

        unsigned char insertBytesBegin[sizeof(command_type) + 1 + argsSize];
        unsigned char* insertBytesEnd = insertBytesBegin;

        // Copy command type into insert bytes array.
        const u16 bigType = rad::endian::native_to_big(static_cast<unsigned short>(type));
        std::memcpy(insertBytesEnd, &bigType, sizeof(bigType));
        insertBytesEnd += sizeof(bigType);

        // Copy argument count into insert bytes array.
        *(insertBytesEnd++) = static_cast<unsigned char>(argsSize);

        // Copy arguments into insert bytes array.
        (
            (args = rad::endian::native_to_big(args),
            std::memcpy(insertBytesEnd, &args, sizeof(args)),
            insertBytesEnd += sizeof(args)), ...
        );

        // Insert the entire contents of the insert bytes array.
        return iterator(rawData.insert(
            pos.as_raw_data_iterator(),
            insertBytesBegin,
            insertBytesEnd
        ));
    }

    iterator insert_no_op(const_iterator pos)
    {
        return iterator(rawData.insert(
            pos.as_raw_data_iterator(),
            sizeof(command_type) + 1,
            0
        ));
    }

    inline void append_no_op()
    {
        insert_no_op(end());
    }

    inline iterator insert_set_pitch(const_iterator pos, s16 pitch)
    {
        return insert(pos, command_type::set_pitch, pitch);
    }

    inline void append_set_pitch(s16 pitch)
    {
        insert_set_pitch(end(), pitch);
    }

    // TODO: This is actually an array of u32s !!!
    inline iterator insert_set_category(const_iterator pos, u32 categoryID)
    {
        return insert(pos, command_type::set_category, categoryID);
    }

    // TODO: This is actually an array of u32s !!!
    inline void append_set_category(u32 categoryID)
    {
        insert_set_category(end(), categoryID);
    }

    inline iterator insert_set_volume(const_iterator pos, u16 volume)
    {
        return insert(pos, command_type::set_volume, volume);
    }

    inline void append_set_volume(u16 volume)
    {
        insert_set_volume(end(), volume);
    }

    inline iterator insert_set_bus_send(
        const_iterator pos,
        u16 nameStringValIndex,
        u16 volume)
    {
        return insert(pos, command_type::set_bus_send, nameStringValIndex, volume);
    }

    inline void append_set_bus_send(
        u16 nameStringValIndex,
        u16 volume)
    {
        insert_set_bus_send(end(), nameStringValIndex, volume);
    }

    inline iterator insert_seq_loop_end(
        const_iterator pos,
        u16 loopIndex,
        u32 loopDurationMilliseconds)
    {
        return insert(pos, command_type::seq_loop_end, loopIndex, loopDurationMilliseconds);
    }

    inline void append_seq_loop_end(
        u16 loopIndex,
        u32 loopDurationMilliseconds)
    {
        insert_seq_loop_end(end(), loopIndex, loopDurationMilliseconds);
    }

    inline iterator insert_seq_loop_start(
        const_iterator pos,
        u16 loopIndex,
        u16 loopRepeatCount = UINT16_MAX)
    {
        return insert(pos, command_type::seq_loop_start, loopIndex, loopRepeatCount);
    }

    inline void append_seq_loop_start(
        u16 loopIndex,
        u16 loopRepeatCount = UINT16_MAX)
    {
        insert_seq_loop_start(end(), loopIndex, loopRepeatCount);
    }

    inline iterator insert_play(
        const_iterator pos,
        reference_type refType,
        u16 refIndex)
    {
        return insert(pos, command_type::play, static_cast<u16>(refType), refIndex);
    }

    inline void append_play(
        reference_type refType,
        u16 refIndex)
    {
        insert_play(end(), refType, refIndex);
    }

    inline iterator insert_wait(
        const_iterator pos,
        u32 numMilliseconds)
    {
        return insert(pos, command_type::wait, numMilliseconds);
    }

    inline void append_wait(u32 numMilliseconds)
    {
        insert_wait(end(), numMilliseconds);
    }

    HL_API u16 compute_related_waveform_count(const cue_sheet& cueSheet) const;

    HL_API void write(rad::stream& stream) const;

    command_table() noexcept = default;

    HL_API command_table(rad::vector<unsigned char> data) noexcept;
};

class aisac_control
{
public:
    rad::string name;
    u16         id;

    inline aisac_control(rad::string name, u16 id) noexcept
        : name(std::move(name))
        , id(id)
    {
    }
};

struct graph_point
{
    float   control;
    u16     destination;
    u16     curve = 100;

    constexpr graph_point(
        float control,
        u16 destination,
        u16 curve = 100) noexcept
        : control(control)
        , destination(destination)
        , curve(curve)
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

class graph
{
public:
    graph_type                  type;
    rad::vector<graph_point>    points;
};

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

class auto_modulation
{
public:
    auto_modulation_type    type = auto_modulation_type::loop;
    trigger_type            triggerType = trigger_type::playback;

    /// @brief Time in milliseconds.
    u32                     time;

    /// @brief Used if triggerType == key.
    u32                     key = UINT32_MAX;
};

enum class aisac_type : u8
{
    simple = 0,
    automation = 1, // TODO: Is this a good name for this?
};

class aisac
{
public:
    /// @brief TODO: What is this value for? It seems to never be set to anything but 0.
    s16                     id = 0;
    aisac_type              type = aisac_type::simple;

    // TODO: This seems to be 1000 + ID if type == simple, 
    // 2000 + something if type == automation,
    // 3000 + something if auto modulation is used
    u16                     controlID = 0;

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

    /// @brief Platform-specific codec used by 3DS.
    /// @details HW1 for 3DS? Used in Sonic Lost World for 3DS.
    cwav = 9,

    /// @brief Platform-specific codec used by PS Vita, PS4, and PS5.
    /// @details HW2 for PS Vita? Used in Persona 4 Golden for PS Vita.
    atrac9 = 11,
};

enum waveform_loop_flag : u8
{
    WAVEFORM_LOOP_FLAG_UNKNOWN0 = 0,
    WAVEFORM_LOOP_FLAG_UNKNOWN1 = 1,
    WAVEFORM_LOOP_FLAG_UNKNOWN2 = 2,
};

class waveform
{
public:
    u32                         awbId;
    waveform_encode_type        encodeType;
    bool                        isStreaming;
    u8                          channelCount;
    waveform_loop_flag          loopFlags;
    u32                         sampleRate;
    u32                         sampleCount;
    // TODO
};

class track
{
public:
    unsigned short              eventIndex = UINT16_MAX;
    unsigned short              commandIndex = UINT16_MAX;
    rad::vector<u16>            localAisacIndices;
    rad::vector<rad::string>    globalAisacs;

    HL_API u16 compute_related_waveform_count(const cue_sheet& cueSheet) const;
};

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

class sequence
{
public:
    u16                         playbackRatio = 100;
    rad::vector<u16>            trackIndices;
    u16                         commandIndex = UINT16_MAX;
    rad::vector<u16>            localAisacIndices;
    rad::vector<rad::string>    globalAisacs;
    rad::vector<u16>            trackValues;
    sequence_type               type = sequence_type::polyphonic;
    // TODO

    HL_API u16 compute_related_waveform_count(const cue_sheet& cueSheet) const;
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

class synth
{
public:
    synth_type                      type = synth_type::polyphonic;
    rad::string                     voiceLimitGroupName;
    u16                             commandIndex = UINT16_MAX;
    rad::vector<reference_item>     refItems;
    rad::vector<u16>                localAisacIndices;
    rad::vector<rad::string>        globalAisacs;
    rad::vector<u16>                trackValues;

    HL_API u16 compute_related_waveform_count(const cue_sheet& cueSheet) const;
};

class cue
{
public:
    u32                             id = 0;
    reference_item                  refItem;
    rad::string                     name;
    rad::string                     userData;
    
    /// @brief Bit array, where each bit represents whether the
    /// AISAC Control at the corresponding index is used by the
    /// cue or not.
    rad::stack_or_heap_array<u8, 8> aisacControlMap;

    /// @brief How many milliseconds this cue should play for,
    /// or UINT32_MAX to loop indefinitely.
    u32                             playDuration = UINT32_MAX;

    // TODO: Expose HeaderVisibility
    
    inline u16 compute_related_waveform_count(const cue_sheet& cueSheet) const
    {
        return refItem.compute_related_waveform_count(cueSheet);
    }
};

enum class config_reference_item_type : u8
{
    category = 3,
    aisac = 4,
    aisac_control = 5,
    voice_limit_group = 6,
    dsp_bus = 9,
};

class config_reference_item
{
public:
    config_reference_item_type     type;
    rad::string                 name;
    rad::string                 name2;
    unsigned long               id;

    HL_API config_reference_item(
        config_reference_item_type type,
        rad::string name,
        rad::string name2 = rad::string(),
        unsigned long id = UINT32_MAX
    ) noexcept;
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

        struct
        {
            //u16                 synth = 0;
            u16                 track = 0;
            u16                 sequence = 0;
        }
        globalAisacStartIndices;

        inline unsigned short get_command_index(
            unsigned short commandIndex) noexcept
        {
            // TODO:
            return commandIndex;
            //return (!useGlobalCmdTable || commandIndex == UINT16_MAX) ?
                //commandIndex : globalCmdTableIndex++;
        }
    };

    void validate_reference_type(u8 type);

    void write_aisac_table(write_params& wp, const rad::vector<aisac>& aisacs);

    void write_aisac_control_name_table(
        write_params& wp,
        const rad::vector<aisac_control>& aisacControls
    );

    void write_auto_modulation_table(
        write_params& wp,
        const rad::vector<auto_modulation>& autoModulations
    );

    void write_graph_table(write_params& wp, const rad::vector<graph>& graphs);
}

class cue_sheet
{
    u16 compute_global_aisac_start_indices_(detail_::write_params& wp) const;

    void write_string_value_table_(detail_::write_params& wp) const;

    void write_waveform_table_(detail_::write_params& wp) const;

    void write_synth_table_(detail_::write_params& wp) const;

    void write_track_table_(detail_::write_params& wp) const;

    void write_sequence_table_(detail_::write_params& wp) const;

    void write_cue_name_table_(detail_::write_params& wp) const;

    void read_cue_table_(
        rad::stream& stream,
        packed_version version
    );

    void write_cue_table_(detail_::write_params& wp) const;

    void write_global_aisac_reference_table_(detail_::write_params& wp) const;

    void write_acf_reference_table_(detail_::write_params& wp) const;

    void write_stream_awb_hash_table_(
        detail_::write_params& wp,
        rad::span<const unsigned char> streamingAwbHash
    ) const;

    static void write_stream_awb_header_table_(
        detail_::write_params& wp,
        rad::span<const unsigned char> streamingAwbToc
    );

    void read_columns_ex_(
        utf::table_deserializer& td,
        packed_version version
    );

public:
    static constexpr packed_version latest_supported_version = packed_version(1, 42, 01);

    rad::string                         name;
    rad::vector<rad::string>            stringValues;
    rad::vector<aisac_control>          aisacControls;
    rad::vector<graph>                  graphs;
    rad::vector<auto_modulation>        autoModulations;
    rad::vector<aisac>                  aisacs;
    rad::vector<waveform>               waveforms;
    rad::vector<command_table>          trackEventCmdTables;
    rad::vector<command_table>          trackCmdTables;
    rad::vector<track>                  tracks;
    rad::vector<command_table>          sequenceCmdTables;
    rad::vector<sequence>               sequences;
    rad::vector<command_table>          synthCmdTables;
    rad::vector<synth>                  synths;
    rad::vector<cue>                    cues;
    //rad::vector<rad::string>            cueNames;
    rad::vector<config_reference_item>  acfRefItems;
    unsigned char                       acfMD5Hash[16] = {};

    HL_API bool has_any_command_tables() const noexcept;

    HL_API void clear() noexcept;

    HL_API void read_inner(rad::stream& stream);

    HL_API void read(rad::stream& stream);

    HL_API void write(
        rad::stream& stream,
        rad::span<const unsigned char> embeddedAwb,
        rad::span<const unsigned char> streamingAwbToc = nullptr,
        packed_version version = latest_supported_version,
        utf::encoding_type encoding = utf::encoding_type::utf8
    ) const;

    constexpr cue_sheet() noexcept = default;

    HL_API explicit cue_sheet(rad::stream& stream);
};
}

#endif
