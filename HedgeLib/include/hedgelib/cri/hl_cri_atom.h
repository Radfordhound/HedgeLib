#ifndef HL_CRI_ATOM_H_INCLUDED
#define HL_CRI_ATOM_H_INCLUDED

#include <array>

#include <rad/rad_endian.h>
#include <rad/rad_vector.h> // TODO: Switch command_table to memory stream ?
//#include <rad/rad_memory_stream.h>

#include "../hl_internal.h"

namespace hl::cri::atom
{
using md5_hash = std::array<unsigned char, 16>;

enum class encoding_type : u8
{
    utf8 = 0,
    shift_jis = 1,
};

enum class ref_type : u8
{
    waveform = 1,
    synth = 2,
    sequence = 3,
    block_sequence = 8,
    //none = 255, // TODO: Is this really used anywhere?? I can't remember, but it looks like something I just made up!
};

struct ref_item
{
    ref_type        type;
    u16             index;

    constexpr ref_item(
        ref_type type,
        u16 index) noexcept
        : type(type)
        , index(index)
    {
    }
};

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

enum class command_type : u16
{
    /// @brief Does nothing.
    /// @details no_op()
    no_op = 0x0000,

    /// @brief Sets the pitch to the given level.
    /// @details set_pitch(s16 pitch)
    /// @param pitch A value between [-1200 - 1200] which represents the new pitch.
    set_pitch = 0x0005,

    set_pan3d_volume = 0x0010,

    set_eg_attack = 0x0024,
    set_eg_hold = 0x0025,
    set_eg_decay = 0x0026,
    set_eg_sustain = 0x0027,
    set_eg_release = 0x0028,

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
    /// @param refType A `ref_type` value representing what type of item is being referenced.
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

    inline iterator insert_set_pan3d_volume(const_iterator pos, u16 volume)
    {
        return insert(pos, command_type::set_pan3d_volume, volume);
    }

    inline void append_set_pan3d_volume(u16 volume)
    {
        insert_set_pan3d_volume(end(), volume);
    }

    inline iterator insert_set_eg_attack(
        const_iterator pos,
        u16 durationMilliseconds,
        u8 curveType = 0, // TODO: Make this an enum
        u8 curveStrength = 100)
    {
        return insert(
            pos,
            command_type::set_eg_attack,
            durationMilliseconds,
            curveType,
            curveStrength
        );
    }

    inline void append_set_eg_attack(
        u16 durationMilliseconds,
        u8 curveType = 0, // TODO: Make this an enum
        u8 curveStrength = 100)
    {
        insert_set_eg_attack(
            end(),
            durationMilliseconds,
            curveType,
            curveStrength
        );
    }

    inline iterator insert_set_eg_hold(const_iterator pos, u32 durationMilliseconds)
    {
        return insert(pos, command_type::set_eg_hold, durationMilliseconds);
    }

    inline void append_set_eg_hold(u32 durationMilliseconds)
    {
        insert_set_eg_hold(end(), durationMilliseconds);
    }

    inline iterator insert_set_eg_decay(
        const_iterator pos,
        u16 durationMilliseconds,
        u8 curveType = 0, // TODO: Make this an enum
        u8 curveStrength = 100)
    {
        return insert(
            pos,
            command_type::set_eg_decay,
            durationMilliseconds,
            curveType,
            curveStrength
        );
    }

    inline void append_set_eg_decay(
        u16 durationMilliseconds,
        u8 curveType = 0, // TODO: Make this an enum
        u8 curveStrength = 100)
    {
        insert_set_eg_decay(
            end(),
            durationMilliseconds,
            curveType,
            curveStrength
        );
    }

    inline iterator insert_set_eg_sustain(const_iterator pos, u16 sustainLevel)
    {
        return insert(pos, command_type::set_eg_sustain, sustainLevel);
    }

    inline void append_set_eg_sustain(u16 sustainLevel)
    {
        insert_set_eg_sustain(end(), sustainLevel);
    }

    inline iterator insert_set_eg_release(
        const_iterator pos,
        u16 durationMilliseconds,
        u8 curveType = 0, // TODO: Make this an enum
        u8 curveStrength = 100)
    {
        return insert(
            pos,
            command_type::set_eg_release,
            durationMilliseconds,
            curveType,
            curveStrength
        );
    }

    inline void append_set_eg_release(
        u16 durationMilliseconds,
        u8 curveType = 0, // TODO: Make this an enum
        u8 curveStrength = 100)
    {
        insert_set_eg_release(
            end(),
            durationMilliseconds,
            curveType,
            curveStrength
        );
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
        ref_type refType,
        u16 refIndex)
    {
        return insert(pos, command_type::play, static_cast<u16>(refType), refIndex);
    }

    inline void append_play(
        ref_type refType,
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

    HL_API void write(rad::stream& stream) const;

    HL_API command_table(rad::allocator& allocator = rad::default_allocator) noexcept;

    HL_API command_table(rad::vector<unsigned char> data) noexcept;
};
}

#endif
