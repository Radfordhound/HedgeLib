#ifndef HL_GUID_H_INCLUDED
#define HL_GUID_H_INCLUDED
#include "hl_text.h"
#include <string_view>
#include <array>

namespace hl
{
struct guid
{
    std::array<unsigned char, 16> data;

    static constexpr guid zero() noexcept
    {
        return guid(nullptr);
    }

    /**
     * @brief Generates a random guid and returns it.
     * 
     * @return guid The randomly-generated guid.
     */
    HL_API static guid random();

    template<bool swapOffsets = true>
    void endian_swap() noexcept
    {
        // TODO: Should we swap data as though it's the Windows GUID struct?
    }

    [[nodiscard]] constexpr bool empty() const noexcept
    {
        // TODO: SIMD optimizations?
        return (
            !data[0] && !data[1] && !data[2] && !data[3] &&
            !data[4] && !data[5] && !data[6] && !data[7] &&
            !data[8] && !data[9] && !data[10] && !data[11] &&
            !data[12] && !data[13] && !data[14] && !data[15]);
    }

    std::string as_string() const;

    /** @brief Constructs an uninitialized guid; please set its values manually. */
    inline guid() noexcept = default;

    /** @brief Constructs a "null" (empty) guid; that is, a guid with all values set to 0. */
    constexpr guid(std::nullptr_t) noexcept :
        data{} {}

    /**
     * @brief Constructs a guid from the bytes in the given array.
     * 
     * @param arr The array of bytes to construct the guid from.
     */
    constexpr guid(const unsigned char(&arr)[16]) noexcept :
        data{arr[0], arr[1], arr[2], arr[3], arr[4], arr[5],
            arr[6], arr[7], arr[8], arr[9], arr[10], arr[11],
            arr[12], arr[13], arr[14], arr[15]} {}

    constexpr guid(unsigned char v1, unsigned char v2,
        unsigned char v3, unsigned char v4,
        unsigned char v5, unsigned char v6,
        unsigned char v7, unsigned char v8,
        unsigned char v9, unsigned char v10,
        unsigned char v11, unsigned char v12,
        unsigned char v13, unsigned char v14,
        unsigned char v15, unsigned char v16) noexcept :

        data{v1, v2, v3, v4, v5, v6, v7, v8, v9,
            v10, v11, v12, v13, v14, v15, v16} {}

    /**
     * @brief Constructs a guid from a string.
     * 
     * Any of the following formats are accepted:
     * 
     * - 32 case-insensitive hex digits without dashes:
     *      "6B29FC40CA471067B31D00DD010662DA"
     * 
     * - 32 case-insensitive hex digits separated by dashes:
     *      "6B29FC40-CA47-1067-B31D-00DD010662DA"
     * 
     * - 32 case-insensitive hex digits separated by dashes
     *   and surrounded by curly brackets or parenthesis:
     *      "{6B29FC40-CA47-1067-B31D-00DD010662DA}"
     *      "(6B29FC40-CA47-1067-B31D-00DD010662DA)"
     * 
     * @param str The string to construct the guid from.
     */
    constexpr explicit guid(std::string_view str) :
        data(in_string_to_bytes(str)) {}

private:
    static constexpr unsigned char in_hex_to_byte(char ch)
    {
        // Convert '0'-'9' to 0-9
        if (ch >= '0' && ch <= '9')
            return (ch - '0');

        // Convert 'A'-'F' to 10-15
        if (ch >= 'A' && ch <= 'F')
            return (ch - 'A' + 10);

        // Convert 'a'-'f' to 10-15
        if (ch >= 'a' && ch <= 'f')
            return (ch - 'a' + 10);

        throw std::runtime_error("Invalid GUID string");
    }

    static constexpr unsigned char in_hex_to_byte(char a, char b)
    {
        // Convert two hex digits to one byte.
        // (e.g. "1C" -> 28)
        return ((in_hex_to_byte(a) * 16) + in_hex_to_byte(b));
    }

    static constexpr std::array<unsigned char, 16>
        in_string_to_bytes_no_dash(const char* str)
    {
        // Convert GUID string to bytes.
        return {{
            in_hex_to_byte(str[0], str[1]),
            in_hex_to_byte(str[2], str[3]),
            in_hex_to_byte(str[4], str[5]),
            in_hex_to_byte(str[6], str[7]),
            in_hex_to_byte(str[8], str[9]),
            in_hex_to_byte(str[10], str[11]),
            in_hex_to_byte(str[12], str[13]),
            in_hex_to_byte(str[14], str[15]),
            in_hex_to_byte(str[16], str[17]),
            in_hex_to_byte(str[18], str[19]),
            in_hex_to_byte(str[20], str[21]),
            in_hex_to_byte(str[22], str[23]),
            in_hex_to_byte(str[24], str[25]),
            in_hex_to_byte(str[26], str[27]),
            in_hex_to_byte(str[28], str[29]),
            in_hex_to_byte(str[30], str[31])
        }};
    }

    static constexpr std::array<unsigned char, 16>
        in_string_to_bytes_dash(const char* str)
    {
        // Verify dashes are in the expected positions.
        if (str[8] != '-' || str[13] != '-' ||
            str[18] != '-' || str[23] != '-')
        {
            throw std::runtime_error("Invalid GUID string");
        }

        // Convert GUID string to bytes.
        return {{
            in_hex_to_byte(str[0], str[1]),
            in_hex_to_byte(str[2], str[3]),
            in_hex_to_byte(str[4], str[5]),
            in_hex_to_byte(str[6], str[7]),
            // -
            in_hex_to_byte(str[9], str[10]),
            in_hex_to_byte(str[11], str[12]),
            // -
            in_hex_to_byte(str[14], str[15]),
            in_hex_to_byte(str[16], str[17]),
            // -
            in_hex_to_byte(str[19], str[20]),
            in_hex_to_byte(str[21], str[22]),
            // -
            in_hex_to_byte(str[24], str[25]),
            in_hex_to_byte(str[26], str[27]),
            in_hex_to_byte(str[28], str[29]),
            in_hex_to_byte(str[30], str[31]),
            in_hex_to_byte(str[32], str[33]),
            in_hex_to_byte(str[34], str[35])
        }};
    }

    static constexpr std::array<unsigned char, 16>
        in_string_to_bytes_dash_enclosed(const char* str)
    {
        // Verify curly brackets/parenthesis are in the expected positions.
        if ((str[0] != '{' || str[37] != '}') &&
            (str[0] != '(' || str[37] != ')'))
        {
            throw std::runtime_error("Invalid GUID string");
        }

        // Convert GUID string to bytes.
        return in_string_to_bytes_dash(str + 1);
    }

    static constexpr std::array<unsigned char, 16>
        in_string_to_bytes(std::string_view str)
    {
        switch (str.size())
        {
        case 38:
            return in_string_to_bytes_dash_enclosed(str.data());

        case 36:
            return in_string_to_bytes_dash(str.data());

        case 32:
            return in_string_to_bytes_no_dash(str.data());
        
        default:
            throw std::runtime_error("Invalid GUID string");
        }
    }
};

namespace internal
{
HL_API bool in_guids_are_equal(const guid& a, const guid& b) noexcept;
} // internal

#ifdef __cpp_lib_is_constant_evaluated
constexpr bool operator==(const guid& a, const guid& b) noexcept
{
    // Compile-time equality check.
    if (std::is_constant_evaluated())
    {
        return a.data == b.data;
    }

    // Runtime SIMD-optimized equality check.
    else
    {
        return internal::in_guids_are_equal(a, b);
    }
}
#else
inline bool operator==(const guid& a, const guid& b) noexcept
{
    return internal::in_guids_are_equal(a, b);
}
#endif
} // hl

namespace std
{
template<>
struct hash<hl::guid>
{
    HL_API size_t operator()(const hl::guid& val) const;
};
} // std
#endif
