#include "hedgelib/hl_guid.h"

#ifdef _WIN32
#include <objbase.h>
#elif defined(__APPLE__)
#include <CoreFoundation/CFUUID.h>
#else
#include <uuid/uuid.h>
#endif

namespace hl
{
guid guid::random()
{
#ifdef _WIN32
    UUID uuid;
    if (FAILED(CoCreateGuid(&uuid)))
    {
        throw std::runtime_error("Could not generate a random guid");
    }

    // NOTE: This is correct regardless of system endianness.
    return guid(
        static_cast<unsigned char>((uuid.Data1 >> 24) & 0xFF),
        static_cast<unsigned char>((uuid.Data1 >> 16) & 0xFF),
        static_cast<unsigned char>((uuid.Data1 >> 8) & 0xFF),
        static_cast<unsigned char>(uuid.Data1 & 0xFF),
        static_cast<unsigned char>((uuid.Data2 >> 8) & 0xFF),
        static_cast<unsigned char>(uuid.Data2 & 0xFF),
        static_cast<unsigned char>((uuid.Data3 >> 8) & 0xFF),
        static_cast<unsigned char>(uuid.Data3 & 0xFF),
        uuid.Data4[0], uuid.Data4[1], uuid.Data4[2], uuid.Data4[3],
        uuid.Data4[4], uuid.Data4[5], uuid.Data4[6], uuid.Data4[7]
    );
#elif defined(__APPLE__)
    // TODO: Change this to use uuid_create(&uuid, &status); from BSD UUID instead?
    auto uuidRef = CFUUIDCreate(NULL);
    const auto uuidBytes = CFUUIDGetUUIDBytes(uuidRef);
    CFRelease(uuidRef);

    return guid(
        uuidBytes.byte0, uuidBytes.byte1, uuidBytes.byte2, uuidBytes.byte3,
        uuidBytes.byte4, uuidBytes.byte5, uuidBytes.byte6, uuidBytes.byte7,
        uuidBytes.byte8, uuidBytes.byte9, uuidBytes.byte10, uuidBytes.byte11,
        uuidBytes.byte12, uuidBytes.byte13, uuidBytes.byte14, uuidBytes.byte15
    );
#else
    if constexpr(std::is_same_v<uuid_t, unsigned char[16]>)
    {
        // Construct the guid directly from the uuid and return it.
        guid val;
        uuid_generate(val.data.data());
        return val;
    }
    else
    {
        // Assume uuid_t is some type that we can
        // index to get the values that we want.
        uuid_t uuid;
        uuid_generate(uuid);

        // Construct a guid from uuid_t values and return it.
        return guid(
            uuid[0], uuid[1], uuid[2], uuid[3],
            uuid[4], uuid[5], uuid[6], uuid[7],
            uuid[8], uuid[9], uuid[10], uuid[11],
            uuid[12], uuid[13], uuid[14], uuid[15]
        );
    }
#endif
}

class in_guid_string_builder
{
    char* m_ptr;

    static char in_nibble_to_hex(unsigned char val) noexcept
    {
        return (val < 10) ?
            static_cast<char>('0' + val) :
            static_cast<char>('A' + val - 10);
    }

public:
    inline void append_char(char ch)
    {
        *(m_ptr)++ = ch;
    }

    inline void append_hex_byte(unsigned char val)
    {
        *(m_ptr++) = in_nibble_to_hex(val / 16);
        *(m_ptr++) = in_nibble_to_hex(val & 0x0F);
    }

    friend in_guid_string_builder& operator<<(
        in_guid_string_builder& builder, unsigned char val);

    inline in_guid_string_builder(char* ptr) noexcept :
        m_ptr(ptr) {}

    inline in_guid_string_builder(std::string& str) noexcept :
        m_ptr(str.data()) {}
};

std::string guid::as_string() const
{
    // TODO: Give user ability to specify string formatting.
    std::string str(38, '\0');
    in_guid_string_builder builder(str);
    builder.append_char('{');

    builder.append_hex_byte(data[0]);
    builder.append_hex_byte(data[1]);
    builder.append_hex_byte(data[2]);
    builder.append_hex_byte(data[3]);

    builder.append_char('-');

    builder.append_hex_byte(data[4]);
    builder.append_hex_byte(data[5]);

    builder.append_char('-');

    builder.append_hex_byte(data[6]);
    builder.append_hex_byte(data[7]);

    builder.append_char('-');

    builder.append_hex_byte(data[8]);
    builder.append_hex_byte(data[9]);

    builder.append_char('-');

    builder.append_hex_byte(data[10]);
    builder.append_hex_byte(data[11]);
    builder.append_hex_byte(data[12]);
    builder.append_hex_byte(data[13]);
    builder.append_hex_byte(data[14]);
    builder.append_hex_byte(data[15]);

    builder.append_char('}');

    return str;
}

namespace internal
{
bool in_guids_are_equal(const guid& a, const guid& b) noexcept
{
    // TODO: SIMD intrinsics support!
    return a.data == b.data;
}
} // internal
} // hl

namespace std
{
size_t hash<hl::guid>::operator()(const hl::guid& val) const
{
    /* 64-bit size_t */
    if constexpr (sizeof(size_t) == (sizeof(unsigned char) * 8))
    {
        auto ptr = reinterpret_cast<const size_t*>(&val.data);
        return (*ptr ^ *(ptr + 1));
    }
    
    /* 32-bit size_t */
    else if constexpr (sizeof(size_t) == (sizeof(unsigned char) * 4))
    {
        auto ptr = reinterpret_cast<const size_t*>(&val.data);
        return (*ptr ^ *(ptr + 1) ^ *(ptr + 2) ^ *(ptr + 3));
    }
    
    /* other */
    else
    {
        return (
            val.data[0] ^ val.data[1] ^ val.data[2] ^ val.data[3] ^
            val.data[4] ^ val.data[5] ^ val.data[6] ^ val.data[7] ^
            val.data[8] ^ val.data[9] ^ val.data[10] ^ val.data[11] ^
            val.data[12] ^ val.data[13] ^ val.data[14] ^ val.data[15]
        );
    }
}
} // std
