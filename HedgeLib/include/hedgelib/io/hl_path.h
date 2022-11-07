#ifndef HL_PATH_H_INCLUDED
#define HL_PATH_H_INCLUDED
#include "../hl_text.h"

namespace hl
{
namespace path
{
#ifdef _WIN32
constexpr const nchar separator = HL_NTEXT('\\');
#else
constexpr const nchar separator = HL_NTEXT('/');
#endif

enum class dir_entry_type : unsigned short
{
    not_found = 0,
    regular,
    directory,
    symlink,
    block,
    character,
    fifo,
    socket,
    unknown,
    junction
};

enum class dir_entry_perms : unsigned short
{
    none = 0,

    owner_read = 0400,
    owner_write = 0200,
    owner_exec = 0100,
    owner_all = 0700,

    group_read = 0040,
    group_write = 0020,
    group_exec = 0010,
    group_all = 0070,

    others_read = 0004,
    others_write = 0002,
    others_exec = 0001,
    others_all = 0007,

    all = 0777,
    set_uid = 04000,
    set_gid = 02000,
    sticky_bit = 01000,
    mask = 07777,
    unknown = 0xFFFF
};

class dir_entry
{
    /** @brief Pointer to this entry's name (e.g. the file or directory name). */
    const nchar* m_name = nullptr;
    dir_entry_type m_type = dir_entry_type::not_found;
    dir_entry_perms m_permissions = dir_entry_perms::unknown;

public:
    inline const nchar* name() const noexcept
    {
        return m_name;
    }

    inline dir_entry_type type() const noexcept
    {
        return m_type;
    }

    HL_API dir_entry_perms permissions() const;

    inline dir_entry() = default;
    inline dir_entry(const nchar* name, dir_entry_type type,
        dir_entry_perms perms) : m_name(name), m_type(type),
        m_permissions(perms) {}
};

class dir
{
    void* m_handle;
    dir_entry m_curEntry = dir_entry();

public:
    HL_API const dir_entry& next();

    class iterator
    {
        dir* m_dir = nullptr;

        friend dir;

        inline iterator() noexcept = default;
        inline iterator(dir* dir) noexcept : m_dir(
            (dir->m_curEntry.type() != dir_entry_type::not_found) ?
            dir : nullptr) {}

    public:
        inline const dir_entry& operator*() const noexcept
        {
            return m_dir->m_curEntry;
        }

        inline const dir_entry* operator->() const noexcept
        {
            return &m_dir->m_curEntry;
        }

        inline iterator& operator++()
        {
            // NOTE: We intentionally don't null-check m_dir here;
            // calling ++ on an "end" hl::path::dir::iterator will
            // invoke undefined behavior, so don't do that, just
            // like how you wouldn't increment a poiner past the 
            // end of an array.
            if (m_dir->next().type() == dir_entry_type::not_found)
            {
                m_dir = nullptr;
            }

            return *this;
        }

        inline bool operator==(const iterator& other) const noexcept
        {
            return (m_dir == other.m_dir);
        }

        inline bool operator!=(const iterator& other) const noexcept
        {
            return (m_dir != other.m_dir);
        }
    };

    inline iterator begin() noexcept
    {
        return iterator(this);
    }

    inline iterator end() noexcept
    {
        return iterator();
    }

    HL_API dir(const nchar* dirPath);

    inline dir(const nstring& dirPath) :
        dir(dirPath.c_str()) {}

    HL_API ~dir();
};

namespace internal
{
template<std::size_t i, typename char_t>
struct in_split_iterator
{
    inline static bool next(char_t* lastCharPtr)
    {
        if (++(*lastCharPtr) > static_cast<char_t>('9'))
        {
            *lastCharPtr = static_cast<char_t>('0');
            return in_split_iterator<i - 1, char_t>::next(lastCharPtr - 1);
        }

        return true;
    }
};

template<typename char_t>
struct in_split_iterator<0, char_t>
{
    inline static bool next(char_t* lastCharPtr)
    {
        // Increment "highest" digit in split extension
        // and return whether it's <= 9.
        return (++(*lastCharPtr) <= static_cast<char_t>('9'));
    }
};
} // internal

template<std::size_t splitCharCount, typename char_t = nchar>
class split_iterator
{
    const char_t* m_filePathPtr = nullptr;
    char_t* m_lastCharPtr = nullptr;

    inline bool in_next() const
    {
        return internal::in_split_iterator<splitCharCount - 1,
            char_t>::next(m_lastCharPtr);
    }

public:
    inline split_iterator begin() noexcept
    {
        return *this;
    }

    inline split_iterator end() const noexcept
    {
        return split_iterator();
    }

    inline const char_t* operator*() const noexcept
    {
        return m_filePathPtr;
    }

    inline split_iterator& operator++()
    {
        if (!in_next())
        {
            m_filePathPtr = nullptr;
            m_lastCharPtr = nullptr;
        }

        return *this;
    }

    inline bool operator==(const split_iterator& other) const noexcept
    {
        return (m_filePathPtr == other.m_filePathPtr);
    }

    inline bool operator!=(const split_iterator& other) const noexcept
    {
        return (m_filePathPtr != other.m_filePathPtr);
    }

    inline split_iterator() noexcept = default;

    inline split_iterator(char_t* filePath, std::size_t filePathLen) noexcept :
        m_filePathPtr(filePath), m_lastCharPtr(&filePath[filePathLen - 1]) {}

    inline split_iterator(char_t* filePath) noexcept :
        split_iterator(filePath, text::len(filePath)) {}

    inline split_iterator(std::basic_string<char_t>& filePath) :
        m_filePathPtr(filePath.c_str()), m_lastCharPtr(&filePath.back()) {}
};

template<typename char_t = nchar>
using split_iterator2 = split_iterator<2, char_t>;

template<typename char_t = nchar>
using split_iterator3 = split_iterator<3, char_t>;

template<typename char_t>
inline std::size_t ext_is_split(const char_t* ext)
{
    std::size_t numSplitChars = 0;
    
    // Account for dot at beginning of extension (if any).
    if (*ext == static_cast<char_t>('.')) ++ext;

    // Ensure there is at least one digit in the extension.
    if (!text::is_digit(*ext)) return 0;

    // Loop through the remaining characters in the extension.
    while (true)
    {
        // Increment numSplitChars.
        ++numSplitChars;

        // Return if we encounter a character which isn't a valid ASCII digit.
        if (!text::is_digit(ext[numSplitChars]))
        {
            // If this non-digit character is the null-terminator, this
            // is a valid split extension. Otherwise, it isn't.
            return (ext[numSplitChars] == static_cast<char_t>('\0')) ?
                numSplitChars : 0;
        }
    }
}

template<typename char_t>
constexpr bool is_path_sep(char_t c) noexcept
{
#ifdef _WIN32
    return (c == '\\' || c == '/');
#else
    // (Don't include backslashes in this check, as paths on
    // POSIX systems allow backslashes in file names.)
    return (c == '/');
#endif
}

template<typename char_t>
inline const char_t* get_name(const char_t* path)
{
    /*
    =======================================
    == Examples:
    == (Null terminator represented with $)
    =======================================
    test.bin$           ->      test.bin$
    \test.bin$          ->      test.bin$
    \$                  ->      $
    \\$                 ->      \$
    \\test.bin$         ->      test.bin$
    /home/rad/a.dds$    ->      a.dds$
    C:\tst\a.dds$       ->      a.dds$
    test.bin\$          ->      test.bin\$
    \test.bin\$         ->      test.bin\$
    /home/rad$          ->      rad$
    /home/rad/$         ->      rad/$
    */

    // Get the name of the file/directory at the given path and return it.
    const char_t* curChar = path;
    while (*curChar)
    {
        /* Account for path separators... */
        if (is_path_sep(*curChar) &&
            (curChar == path ||                             // ...before the name.
            *(curChar + 1) != static_cast<char_t>('\0')))   // ...and after the name.
        {
            path = ++curChar;
            continue;
        }

        ++curChar;
    }

    return path;
}

template<typename char_t>
inline const char_t* get_name(const std::basic_string<char_t>& path)
{
    return get_name(path.c_str());
}

template<typename char_t>
inline const char_t* get_ext(const char_t* path)
{
    /*
    =======================================
    == Examples:
    == (Null terminator represented with $)
    =======================================
    test.bin$           ->      .bin$
    test.ar.00$         ->      .00$
    .ar.00$             ->      .00$
    .bin$               ->      .bin$
    test$               ->      $
    test.$              ->      $
    test..$             ->      $
    test..a$            ->      .a$
    */

    // Find last extension within path, if any.
    const char_t* curChar = path;
    const char_t* ext = 0;

    while (*curChar)
    {
        // If this character is a dot and the next
        // character isn't a dot or a null terminator...
        if (*curChar == static_cast<char_t>('.') && *(curChar + 1) &&
            *(curChar + 1) != static_cast<char_t>('.'))
        {
            // We've found a valid extension!
            ext = curChar;

            // Skip ahead by 2 characters here to avoid checking the
            // next character again, since we've already checked it.
            curChar += 2;
            continue;
        }

        ++curChar;
    }

    return (ext) ? ext : curChar;
}

template<typename char_t>
inline const char_t* get_ext(const std::basic_string<char_t>& path)
{
    return get_ext(path.c_str());
}

template<typename char_t>
inline const char_t* get_exts(const char_t* path)
{
    /*
    =======================================
    == Examples:
    == (Null terminator represented with $)
    =======================================
    test.bin$           ->      .bin$
    test.ar.00$         ->      .ar.00$
    .ar.00$             ->      .ar.00$
    .bin$               ->      .bin$
    test$               ->      $
    test.$              ->      $
    test..$             ->      $
    test..a$            ->      .a$
    */

    // Find file name within path.
    path = get_name(path);

    // Find first extension within name, if any.
    const char_t* curChar = path;
    while (*curChar)
    {
        // If this character is a dot and the next
        // character isn't a dot or the null terminator...
        if (*curChar == static_cast<char_t>('.') && *(curChar + 1) &&
            *(curChar + 1) != static_cast<char_t>('.'))
        {
            // We've found the first valid extension! Return it.
            return curChar;
        }

        ++curChar;
    }

    return curChar;
}

template<typename char_t>
inline const char_t* get_exts(const std::basic_string<char_t>& path)
{
    return get_exts(path.c_str());
}

template<typename char_t>
inline std::size_t remove_ext_no_alloc(const char_t* path, char_t* result = nullptr)
{
    /*
    =======================================
    == Examples:
    == (Null terminator represented with $)
    =======================================
    test.bin$           ->      test$
    test.ar.00$         ->      test.ar$
    .ar.00$             ->      .ar$
    .bin$               ->      $
    */

    // Get extension pointer and compute result buffer length.
    const char_t* ext = get_ext(path);
    const std::size_t resultLen = static_cast<std::size_t>(ext - path);

    // Copy stem into buffer (if provided), set null terminator, and return.
    if (result)
    {
        std::copy(path, path + resultLen, result);
        result[resultLen] = static_cast<char_t>('\0');
    }

    return resultLen;
}

template<typename char_t>
inline std::size_t remove_exts_no_alloc(const char_t* path, char_t* result = nullptr)
{
    /*
    =======================================
    == Examples:
    == (Null terminator represented with $)
    =======================================
    test.bin$           ->      test$
    test.ar.00$         ->      test$
    .ar.00$             ->      $
    .bin$               ->      $
    */

    // Get extension pointer and compute result buffer length.
    const char_t* ext = get_exts(path);
    const std::size_t resultLen = static_cast<std::size_t>(ext - path);

    // Copy stem into buffer (if provided), set null terminator, and return.
    if (result)
    {
        std::copy(path, path + resultLen, result);
        result[resultLen] = static_cast<char_t>('\0');
    }

    return resultLen;
}

template<typename char_t>
inline std::basic_string<char_t> remove_ext(const char_t* path)
{
    /*
    =======================================
    == Examples:
    == (Null terminator represented with $)
    =======================================
    test.bin$           ->      test$
    test.ar.00$         ->      test.ar$
    .ar.00$             ->      .ar$
    .bin$               ->      $
    */

    // Get extension pointer and compute result string length.
    const char_t* ext = get_ext(path);
    const std::size_t resultLen = static_cast<std::size_t>(ext - path);

    // Create copy of stem and return it.
    return std::basic_string<char_t>(path, resultLen);
}

template<typename char_t>
inline std::basic_string<char_t> remove_ext(const std::basic_string<char_t>& path)
{
    return remove_ext(path.c_str());
}

template<typename char_t>
inline std::basic_string<char_t> remove_exts(const char_t* path)
{
    /*
    =======================================
    == Examples:
    == (Null terminator represented with $)
    =======================================
    test.bin$           ->      test$
    test.ar.00$         ->      test$
    .ar.00$             ->      $
    .bin$               ->      $
    */

    // Get extension pointer and compute result string length.
    const char_t* ext = get_exts(path);
    const std::size_t resultLen = static_cast<std::size_t>(ext - path);

    // Create copy of stem and return it.
    return std::basic_string<char_t>(path, resultLen);
}

template<typename char_t>
inline std::basic_string<char_t> remove_exts(const std::basic_string<char_t>& path)
{
    return remove_exts(path.c_str());
}

template<typename char_t>
inline std::size_t get_parent_no_alloc(const char_t* path, char_t* result)
{
    /*
    =======================================
    == Examples:
    == (Null terminator represented with $)
    =======================================
    test.bin$           ->      $
    \test.bin$          ->      \$
    \$                  ->      \$
    \\$                 ->      \$
    \\test.bin$         ->      \\$
    /home/rad/a.dds$    ->      /home/rad/$
    C:\tst\a.dds$       ->      C:\tst\$
    test.bin\$          ->      $
    \test.bin\$         ->      \$
    /home/rad$          ->      /home/$
    /home/rad/$         ->      /home/$
    */

    // Get name pointer and parent length.
    const char_t* name = get_name(path);
    const std::size_t parentLen = static_cast<std::size_t>(name - path);

    // Copy parent into buffer (if provided) and return.
    if (result)
    {
        std::copy(path, path + parentLen, result);
        result[parentLen] = static_cast<char_t>('\0');
    }

    return parentLen;
}

template<typename char_t>
inline std::basic_string<char_t> get_parent(const char_t* path)
{
    /*
    =======================================
    == Examples:
    == (Null terminator represented with $)
    =======================================
    test.bin$           ->      $
    \test.bin$          ->      \$
    \$                  ->      \$
    \\$                 ->      \$
    \\test.bin$         ->      \\$
    /home/rad/a.dds$    ->      /home/rad/$
    C:\tst\a.dds$       ->      C:\tst\$
    test.bin\$          ->      $
    \test.bin\$         ->      \$
    /home/rad$          ->      /home/$
    /home/rad/$         ->      /home/$
    */

    // Get name pointer and parent length.
    const char_t* name = get_name(path);
    const std::size_t parentLen = static_cast<std::size_t>(name - path);

    // Create copy of parent and return it.
    return std::basic_string<char_t>(path, parentLen);
}

template<typename char_t>
inline std::basic_string<char_t> get_parent(const std::basic_string<char_t>& path)
{
    return get_parent(path.c_str());
}

template<typename char_t>
inline bool combine_needs_sep1(const char_t* path1, std::size_t path1Len) noexcept
{
    // We need to add a separator if path1 doesn't end with one.
    return (path1Len >= 1 && !is_path_sep(path1[path1Len - 1]));
}

template<typename char_t>
inline bool combine_needs_sep1(const char_t* path1)
{
    return combine_needs_sep1(path1, text::len(path1));
}

template<typename char_t>
inline bool combine_needs_sep1(const std::basic_string<char_t>& path1) noexcept
{
    return combine_needs_sep1(path1.c_str(), path1.length());
}

template<typename char_t>
inline bool combine_needs_sep2(const char_t* path2) noexcept
{
    // We need to add a separator if path2 does not begin with one.
    return !is_path_sep(*path2);
}

template<typename char_t>
inline bool combine_needs_sep2(const std::basic_string<char_t>& path2) noexcept
{
    return combine_needs_sep2(path2.c_str());
}

template<typename char_t>
inline bool combine_needs_sep(const char_t* path1, const char_t* path2,
    std::size_t path1Len)
{
    return (combine_needs_sep1(path1, path1Len) &&
        combine_needs_sep2(path2));
}

template<typename char_t>
inline bool combine_needs_sep(const char_t* path1, const char_t* path2)
{
    return combine_needs_sep(path1, path2, text::len(path1));
}

template<typename char_t>
inline bool combine_needs_sep(const std::basic_string<char_t>& path1,
    const std::basic_string<char_t>& path2)
{
    return combine_needs_sep(path1.c_str(), path2.c_str(), path1.length());
}

template<typename char_t>
inline std::size_t combine_no_alloc(const char_t* path1, const char_t* path2,
    std::size_t path1Len, std::size_t path2Len, char_t* result)
{
    // Determine whether combining the paths will require a path separator.
    const bool needsSep = combine_needs_sep(path1, path2, path1Len);
    if (needsSep) ++path1Len;

    // Compute the length required to combine the two paths.
    const std::size_t combinedLen = (path1Len + path2Len);

    // Combine the two paths and store the result in the buffer (if provided), then return.
    if (result)
    {
        // Copy path1 into buffer.
        std::copy(path1, path1 + path1Len, result);

        // Append path separator if necessary.
        if (needsSep)
        {
            result[path1Len - 1] = static_cast<char_t>(separator);
        }

        // Copy path2 into buffer. 
        std::copy(path2, path2 + path2Len, result + path1Len);

        // Append null terminator.
        result[combinedLen] = static_cast<char_t>('\0');
    }

    return combinedLen;
}

template<typename char_t>
inline std::size_t combine_no_alloc(const char_t* path1, const char_t* path2,
    char_t* result)
{
    return combine_no_alloc(path1, path2, text::len(path1),
        text::len(path2), result);
}

template<typename char_t>
inline std::basic_string<char_t> combine(const char_t* path1,
    const char_t* path2, std::size_t path1Len, std::size_t path2Len)
{
    // Determine whether combining the paths will require a path separator.
    const bool needsSep = combine_needs_sep(path1, path2, path1Len);
    if (needsSep) ++path1Len;

    // Compute the length required to combine the two paths.
    const std::size_t combinedLen = (path1Len + path2Len);

    // Create a string and reserve enough space for the combined path.
    std::basic_string<char_t> result;
    result.reserve(combinedLen);

    // Append path1.
    result.append(path1, path1Len);

    // Append path separator if necessary.
    if (needsSep)
    {
        result[path1Len - 1] = static_cast<char_t>(separator);
    }

    // Append path2 and return string.
    result.append(path2, path2Len);
    return result;
}

template<typename char_t>
inline std::basic_string<char_t> combine(const char_t* path1, const char_t* path2)
{
    return combine(path1, path2, text::len(path1), text::len(path2));
}

template<typename char_t>
inline std::basic_string<char_t> combine(const std::basic_string<char_t>& path1,
    const std::basic_string<char_t>& path2)
{
    return combine(path1.c_str(), path2.c_str(), path1.length(), path2.length());
}

HL_API std::size_t get_size(const nchar* filePath);

inline std::size_t get_size(const nstring& filePath)
{
    return get_size(filePath.c_str());
}

HL_API bool exists(const nchar* path);

inline bool exists(const nstring& path)
{
    return exists(path.c_str());
}

HL_API void copy_file(const nchar* src, const nchar* dst);

inline void copy_file(const nstring& src, const nstring& dst)
{
    copy_file(src.c_str(), dst.c_str());
}

HL_API bool is_dir(const nchar* path);

inline bool is_dir(const nstring& path)
{
    return is_dir(path.c_str());
}

HL_API void create_dir(const nchar* path, bool overwrite = true);

inline void create_dir(const nstring& path, bool overwrite = true)
{
    create_dir(path.c_str(), overwrite);
}
} // path
} // hl
#endif
