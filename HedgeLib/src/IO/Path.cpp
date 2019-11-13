#include "INPath.h"
#include "../INString.h"
#include "HedgeLib/IO/Path.h"
#include <algorithm>
#include <stdexcept>
#include <system_error>

#ifdef _WIN32
#include <Windows.h>
#else
#include <unistd.h>
#include <sys/stat.h>
#include <cerrno>
#include <sys/types.h>
#include <dirent.h>
#endif

namespace hl
{
    template<typename char_t>
    const char_t* INPathGetNamePtr(const char_t* path)
    {
        // Find file/directory name
        const char_t* curChar = path;
        while (*curChar)
        {
            if (
#ifdef _WIN32
                // Paths on POSIX systems allow backslashes in file names
                *curChar == static_cast<char_t>('\\') ||
#endif
                *curChar == static_cast<char_t>('/'))
            {
                path = ++curChar;
                continue;
            }

            ++curChar;
        }

        return path;
    }

    template const char* INPathGetNamePtr<char>(const char* path);

    const char* PathGetNamePtr(const char* path)
    {
        if (!path) return EmptyString;
        return INPathGetNamePtr(path);
    }

    template<typename char_t, bool multiExt>
    const char_t* INPathGetExtPtrName(const char_t* fileName)
    {
        if (!*fileName) return fileName;

        // Ignore first . in files that begin with . and
        // return an empty string for paths that end with ..
        const char_t* curChar = fileName;
        if (*(curChar++) == static_cast<char_t>('.') &&
            *curChar == static_cast<char_t>('.'))
        {
            fileName = curChar++;
            if (!*curChar) return curChar;
        }

        // Find extension
        bool foundExt = false;
        while (*curChar)
        {
            if (*curChar == static_cast<char_t>('.'))
            {
                if constexpr (multiExt)
                {
                    return curChar;
                }
                else
                {
                    fileName = curChar;
                    foundExt = true;
                }
            }

            ++curChar;
        }

        return (foundExt) ? fileName : curChar;
    }

    template const char* INPathGetExtPtrName<char, true>(const char* fileName);
    template const char* INPathGetExtPtrName<char, false>(const char* fileName);

    const char* PathGetExtPtrName(const char* fileName)
    {
        if (!fileName) return EmptyString;
        return INPathGetExtPtrName(fileName);
    }

    const char* PathGetExtsPtrName(const char* fileName)
    {
        if (!fileName) return EmptyString;
        return INPathGetExtPtrName<char, true>(fileName);
    }

    const char* PathGetExtPtr(const char* filePath)
    {
        // Return extension pointer
        filePath = PathGetNamePtr(filePath);
        return INPathGetExtPtrName(filePath);
    }

    const char* PathGetExtsPtr(const char* filePath)
    {
        // Return extension pointer
        filePath = PathGetNamePtr(filePath);
        return INPathGetExtPtrName<char, true>(filePath);
    }

    template<typename char_t, bool multiExt>
    std::unique_ptr<char_t[]> INPathGetNameNoExtNamePtr(const char_t* fileName)
    {
        // Get extension pointer and name length
        const char_t* ext = INPathGetExtPtrName<char_t, multiExt>(fileName);
        std::size_t nameLen = static_cast<std::size_t>(ext - fileName);

        // Allocate buffer large enough to hold name
        std::unique_ptr<char_t[]> fileNameNoExt = std::unique_ptr<char_t[]>(
            new char_t[nameLen + 1]);

        // Copy name into new buffer and return
        std::copy(fileName, ext, fileNameNoExt.get());
        fileNameNoExt[nameLen] = static_cast<char_t>('\0');

        return fileNameNoExt;
    }

    std::unique_ptr<char[]> PathGetNameNoExtNamePtr(const char* fileName)
    {
        if (!fileName) throw std::invalid_argument("fileName was null");
        return INPathGetNameNoExtNamePtr<char, false>(fileName);
    }

    std::unique_ptr<char[]> PathGetNameNoExtsNamePtr(const char* fileName)
    {
        if (!fileName) throw std::invalid_argument("fileName was null");
        return INPathGetNameNoExtNamePtr<char, true>(fileName);
    }

    template<typename char_t, bool multiExt>
    std::unique_ptr<char_t[]> INPathGetNameNoExtPtr(const char_t* filePath)
    {
        // Get file name
        filePath = INPathGetNamePtr(filePath);

        // Get name without extension
        return INPathGetNameNoExtNamePtr<char_t, multiExt>(filePath);
    }

    std::unique_ptr<char[]> PathGetNameNoExtPtr(const char* filePath)
    {
        if (!filePath) throw std::invalid_argument("filePath was null");
        return INPathGetNameNoExtPtr<char, false>(filePath);
    }

    std::unique_ptr<char[]> PathGetNameNoExtsPtr(const char* filePath)
    {
        if (!filePath) throw std::invalid_argument("filePath was null");
        return INPathGetNameNoExtPtr<char, true>(filePath);
    }

    template<typename char_t>
    std::size_t INPathGetStemRangeName(const char_t* fileName, const char_t*& stemEnd)
    {
        // Get extension pointer
        stemEnd = INPathGetExtPtrName(fileName);

        // Return size
        return static_cast<std::size_t>(stemEnd - fileName);
    }

    std::size_t PathGetStemRangeName(const char* fileName,
        const char*& stemEnd)
    {
        if (!fileName) return 0;
        return INPathGetStemRangeName(fileName, stemEnd);
    }

    template<typename char_t>
    std::size_t INPathGetStemRange(const char_t*& stemStart, const char_t*& stemEnd)
    {
        // Get name pointer
        stemStart = INPathGetNamePtr(stemStart);
        if (!(*stemStart)) return 0;

        // Return stem range
        return INPathGetStemRangeName(stemStart, stemEnd);
    }

    std::size_t PathGetStemRange(const char* path,
        const char*& stemStart, const char*& stemEnd)
    {
        if (!path) return 0;

        stemStart = path;
        return INPathGetStemRange(stemStart, stemEnd);
    }

    template<typename char_t>
    std::unique_ptr<char_t[]> INPathGetStemPtr(const char_t* path)
    {
        // Get stem range
        const char_t* ext;
        std::size_t stemLen = INPathGetStemRange(path, ext);

        // Allocate a buffer large enough to hold stem
        std::unique_ptr<char_t[]> stem = std::unique_ptr<char_t[]>(
            new char_t[stemLen + 1]);

        // Copy stem into new buffer and return
        std::copy(path, ext, stem.get());
        stem[stemLen] = static_cast<char_t>('\0');

        return stem;
    }

    std::unique_ptr<char[]> PathGetStemPtr(const char* path)
    {
        if (!path) throw std::invalid_argument("path was null");
        return INPathGetStemPtr(path);
    }

    template<typename char_t>
    std::unique_ptr<char_t[]> INPathGetParentPtr(const char_t* path,
        const char_t* fileName)
    {
        // Get parent length
        std::size_t parentLen = static_cast<std::size_t>(fileName - path);
        if (parentLen < 2) ++parentLen;

        // Allocate a buffer large enough to hold stem
        std::unique_ptr<char_t[]> parent = std::unique_ptr<char_t[]>(
            new char_t[parentLen]);

        // Copy stem into new buffer and return
        if (parentLen > 1) std::copy(path, fileName, parent.get());
        parent[parentLen - 1] = static_cast<char_t>('\0');
        return parent;
    }

    template std::unique_ptr<char[]> INPathGetParentPtr(const char* path,
        const char* fileName);

    template<typename char_t>
    std::unique_ptr<char_t[]> INPathGetParentPtr(const char_t* path)
    {
        // Get name pointer
        const char_t* fileName = INPathGetNamePtr(path);

        // Return parent
        return INPathGetParentPtr(path, fileName);
    }

    template std::unique_ptr<char[]> INPathGetParentPtr<char>(const char* path);

    std::unique_ptr<char[]> PathGetParentPtr(const char* path)
    {
        if (!path) throw std::invalid_argument("path was null");
        return INPathGetParentPtr(path);
    }

    template<typename char_t>
    void INPathCombineNoAlloc(const char_t* path1, const char_t* path2,
        std::size_t path1Len, std::size_t path2Len, char_t* buffer, bool addSlash)
    {
        // Copy path1 and append slash if necessary
        std::copy(path1, path1 + path1Len, buffer);
        if (addSlash) buffer[path1Len - 1] =
            static_cast<char_t>(PathSeparator);

        // Copy path2
        std::copy(path2, path2 + path2Len, buffer + path1Len);
    }

    template<typename char_t>
    std::unique_ptr<char_t[]> INPathCombinePtr(const char_t* path1, const char_t* path2,
        std::size_t path1Len, std::size_t path2Len)
    {
        // Check if we need to append a slash to the end of path1
        bool addSlash = INPathCombineNeedsSlash(path1, path2, path1Len);
        if (addSlash) ++path1Len;

        // Allocate a buffer large enough to hold result
        std::unique_ptr<char_t[]> result = std::unique_ptr<char_t[]>(
            new char_t[path1Len + ++path2Len]);

        // Combine paths and return
        INPathCombineNoAlloc(path1, path2, path1Len,
            path2Len, result.get(), addSlash);

        return result;
    }

    template<typename char_t>
    std::unique_ptr<char_t[]> INPathCombinePtr(const char_t* path1, const char_t* path2)
    {
        // Determine path lengths
        std::size_t path1Len = StringLength(path1);
        std::size_t path2Len = StringLength(path2);

        // Combine paths
        return INPathCombinePtr(path1, path2,
            path1Len, path2Len);
    }

    std::unique_ptr<char[]> PathCombinePtr(const char* path1, const char* path2)
    {
        if (!path1 || !path2 || (!*path1 && !*path2))
            throw std::invalid_argument("path1 and/or path2 was null or empty");

        return INPathCombinePtr(path1, path2);
    }

    template<typename char_t, bool multiExt>
    std::unique_ptr<char_t[]> INPathRemoveExtPtr(const char_t* filePath)
    {
        // Get file name and first extension
        const char_t* fileName = INPathGetNamePtr<char_t>(filePath);
        const char_t* ext = INPathGetExtPtrName<char_t, multiExt>(fileName);
        std::size_t pathLen = static_cast<std::size_t>(ext - filePath);

        // Allocate a buffer large enough to hold path
        std::unique_ptr<char_t[]> pathNoExt = std::unique_ptr<char_t[]>(
            new char_t[pathLen + 1]);

        // Copy stem into new buffer and return
        std::copy(filePath, ext, pathNoExt.get());
        pathNoExt[pathLen] = static_cast<char_t>('\0');

        return pathNoExt;
    }

    template std::unique_ptr<char[]> INPathRemoveExtPtr<char, true>(const char* filePath);
    template std::unique_ptr<char[]> INPathRemoveExtPtr<char, false>(const char* filePath);

    std::unique_ptr<char[]> PathRemoveExtPtr(const char* filePath)
    {
        if (!filePath) throw std::invalid_argument("filePath was null");
        return INPathRemoveExtPtr<char, false>(filePath);
    }

    std::unique_ptr<char[]> PathRemoveExtsPtr(const char* filePath)
    {
        if (!filePath) throw std::invalid_argument("filePath was null");
        return INPathRemoveExtPtr<char, true>(filePath);
    }

    bool INPathIsDirectory(const nchar* path)
    {
#ifdef _WIN32
        DWORD attrs = GetFileAttributesW(path);
        return (attrs != INVALID_FILE_ATTRIBUTES &&
            (attrs & FILE_ATTRIBUTE_DIRECTORY));
#else
        struct stat sb;
        return (!stat(path, &sb) && S_ISDIR(sb.st_mode));
#endif
    }

    bool PathIsDirectory(const char* path)
    {
#ifdef _WIN32
        // Convert UTF-8 path to wide UTF-16 path
        std::unique_ptr<nchar[]> nativePth = StringConvertUTF8ToNativePtr(path);

        // Check if the given path is a directory
        return INPathIsDirectory(nativePth.get());
#else
        if (!path) throw std::invalid_argument("path was null");
        return INPathIsDirectory(path);
#endif
    }

    bool INPathExists(const nchar* path)
    {
#ifdef _WIN32
        return (GetFileAttributesW(path) != INVALID_FILE_ATTRIBUTES);
#else
        return (access(path, F_OK) != -1);
#endif
    }

    bool PathExists(const char* path)
    {
        if (!path) return false;

#ifdef _WIN32
        // Convert path from UTF-8 to a native (UTF-16) path
        std::unique_ptr<nchar[]> nativePth = StringConvertUTF8ToNativePtr(path);

        // Check if path exists
        return INPathExists(nativePth.get());
#else
        // Check if path exists
        return INPathExists(path);
#endif
    }

    std::size_t INPathGetSize(const nchar* filePath)
    {
#ifdef _WIN32
        WIN32_FILE_ATTRIBUTE_DATA fd;
        if (!GetFileAttributesExW(filePath, GetFileExInfoStandard, &fd))
        {
            std::error_code errorCode(GetLastError(), std::system_category());
            throw std::system_error(errorCode);
        }

        LARGE_INTEGER s;
        s.HighPart = fd.nFileSizeHigh;
        s.LowPart = fd.nFileSizeLow;
        return static_cast<std::size_t>(s.QuadPart);
#else
        struct stat s;
        if (stat(filePath, &s) == -1)
        {
            throw std::runtime_error("stat failed"); // TODO: Give a more helpful error
        }

        return static_cast<std::size_t>(s.st_size);
#endif
    }

    std::size_t PathGetSize(const char* filePath)
    {
#ifdef _WIN32
        std::unique_ptr<nchar[]> nativePth = StringConvertUTF8ToNativePtr(filePath);
        return INPathGetSize(nativePth.get());
#else
        if (!filePath) throw std::invalid_argument("filePath was null");
        return INPathGetSize(filePath);
#endif
    }

    std::size_t INPathGetFileCount(const nchar* dir, bool recursive)
    {
        std::size_t fileCount = 0;

#ifdef _WIN32
        // Adjust the path for usage in the FindFile functions
        // (The Win32 API is the messiest thing I swear)
        std::size_t dirLen = StringLength(dir);
        std::unique_ptr<nchar[]> fdir = std::unique_ptr<nchar[]>(
            new nchar[dirLen + 3]);

        // Copy the actual path
        std::copy(dir, dir + dirLen, fdir.get());

        // We also have to append \* since otherwise windows will just give
        // us the directory rather than files *IN* the directory
        fdir[dirLen] = L'\\';
        fdir[dirLen + 1] = L'*';
        fdir[dirLen + 2] = L'\0';

        // Find the first file in the directory
        WIN32_FIND_DATAW fd;
        HANDLE h = FindFirstFileW(fdir.get(), &fd);

        if (h == INVALID_HANDLE_VALUE)
        {
            // TODO: Return a more helpful error (using GetLastError()?)
            throw std::runtime_error("FindFirstFileW failed");
        }

        // Loop through subsequent files in directory
        try
        {
            do
            {
#else
        // Open the directory
        DIR* d = opendir(dir);
        if (!d)
        {
            // TODO: Use errno to get a more helpful error
            throw std::runtime_error("opendir failed");
        }

        // Loop through subsequent files in directory
        try
        {
            struct dirent* e;
            while ((e = readdir(d))) // Parenthesis here to silence warning
            {
#endif
                // Get file name
                nchar* fileName;

#ifdef _WIN32
                // TODO: Is cFileName always null-terminated?
                fileName = fd.cFileName;
#else
                fileName = e->d_name;
#endif

                // Directories
#ifdef _WIN32
                if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
#else
                if (e->d_type == DT_DIR)
#endif
                {
                    if (recursive)
                    {
                        // Skip . and .. (necessary to avoid infinite recursion)
                        if (StringsEqual(fileName, HL_NTEXT(".")) ||
                            StringsEqual(fileName, HL_NTEXT("..")))
                            continue;

                        // Recurse through subdirectories
                        std::unique_ptr<nchar[]> subDir = INPathCombinePtr(dir, fileName);
                        fileCount += INPathGetFileCount(subDir.get(), recursive);
                    }
                }

                // Files
                // TODO: Handle special cases
                else
//#ifdef _WIN32
//                else if (fd.dwFileAttributes == FILE_ATTRIBUTE_NORMAL)
//#else
//                else if (e->d_type == DT_REG)
//#endif
                {
                    // Increase file count
                    ++fileCount;
                }
            }
#ifdef _WIN32
            while (FindNextFileW(h, &fd));
#endif
        }
        catch (std::exception& ex)
        {
            // Clost the directory
#ifdef _WIN32
            FindClose(h); // TODO: Check result?
#else
            closedir(d); // TODO: Check result?
#endif

            // Re-throw the exception
            throw ex;
        }

        // TODO: Use GetLastError to ensure we've just reached the last file and no error occured.

        // Close the directory
#ifdef _WIN32
        FindClose(h); // TODO: Check result?
#else
        // Close the directory
        closedir(d); // TODO: Check result?
#endif

        return fileCount;
    }

    std::size_t PathGetFileCount(const char* dir, bool recursive)
    {
#ifdef _WIN32
        std::unique_ptr<nchar[]> nativePth = StringConvertUTF8ToNativePtr(dir);
        return INPathGetFileCount(nativePth.get(), recursive);
#else
        if (!dir) throw std::invalid_argument("dir was null");
        return INPathGetFileCount(dir, recursive);
#endif
    }

    void INPathGetFilesInDirectory(const nchar* dir, bool recursive,
        std::vector<std::unique_ptr<nchar[]>>& files)
    {
        std::size_t dirLen = StringLength(dir);

#ifdef _WIN32
        // Adjust the path for usage in the FindFile functions
        // (The Win32 API is the messiest thing I swear)
        std::unique_ptr<nchar[]> fdir = std::unique_ptr<nchar[]>(
            new nchar[dirLen + 3]);

        // Copy the actual path
        std::copy(dir, dir + dirLen, fdir.get());

        // We also have to append \* since otherwise windows will just give
        // us the directory rather than files *IN* the directory
        fdir[dirLen] = L'\\';
        fdir[dirLen + 1] = L'*';
        fdir[dirLen + 2] = L'\0';

        // Find the first file in the directory
        WIN32_FIND_DATAW fd;
        HANDLE h = FindFirstFileW(fdir.get(), &fd);

        if (h == INVALID_HANDLE_VALUE)
        {
            // TODO: Return a more helpful error (using GetLastError()?)
            throw std::runtime_error("FindFirstFileW failed");
        }

        // Loop through subsequent files in directory
        try
        {
            do
            {
#else
        // Open the directory
        DIR* d = opendir(dir);
        if (!d)
        {
            // TODO: Use errno to get a more helpful error
            throw std::runtime_error("opendir failed");
        }

        // Loop through subsequent files in directory
        try
        {
            struct dirent* e;
            while ((e = readdir(d)))
            {
#endif
                // Get file name
                nchar* fileName;

#ifdef _WIN32
                // TODO: Is cFileName always null-terminated?
                fileName = fd.cFileName;
#else
                fileName = e->d_name;
#endif

                // Directories
#ifdef _WIN32
                if (fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
#else
                if (e->d_type == DT_DIR)
#endif
                {
                    // Skip . and .. (necessary to avoid infinite recursion)
                    if (StringsEqual(fileName, HL_NTEXT(".")) ||
                        StringsEqual(fileName, HL_NTEXT("..")))
                        continue;

                    if (recursive)
                    {
                        // Recurse through subdirectories
                        std::unique_ptr<nchar[]> subDir = INPathCombinePtr(dir, fileName);
                        INPathGetFilesInDirectory(subDir.get(), recursive, files);
                    }
                }

                // Files
                // TODO: Handle special cases
                else
//#ifdef _WIN32
//                else if (fd.dwFileAttributes == FILE_ATTRIBUTE_NORMAL)
//#else
//                else if (e->d_type == DT_REG)
//#endif
                {
                    // Copy file name
                    std::size_t nameLen = StringLength(fileName);
                    files.push_back(INPathCombinePtr(dir, fileName, dirLen, nameLen));
                }
            }
#ifdef _WIN32
            while (FindNextFileW(h, &fd));
#endif
        }
        catch (std::exception & ex)
        {
            // Close the directory
#ifdef _WIN32
            FindClose(h); // TODO: Check result?
#else
            closedir(d); // TODO: Check result?
#endif

            // Re-throw the exception
            throw ex;
        }

        // TODO: Use GetLastError to ensure we've just reached the last file and no error occured.

        // Close the directory
#ifdef _WIN32
        FindClose(h); // TODO: Check result?
#else
        // Close the directory
        closedir(d); // TODO: Check result?
#endif
    }

    std::vector<std::unique_ptr<char[]>> INPathGetFilesInDirectoryUTF8(
        const nchar* dir, bool recursive)
    {
#ifdef _WIN32
        // Get files in the given directory as UTF-16 paths
        std::vector<std::unique_ptr<nchar[]>> files;
        INPathGetFilesInDirectory(dir, recursive, files);

        // Convert UTF-16 file paths to UTF-8 and return
        std::vector<std::unique_ptr<char[]>> u8files = std::vector<
            std::unique_ptr<char[]>>(files.size());

        for (std::size_t i = 0; i < files.size(); ++i)
        {
            u8files[i] = StringConvertUTF16ToUTF8Ptr(reinterpret_cast<
                const char16_t*>(files[i].get()));
        }

        return u8files;
#else
        // Get files in the given directory and return
        std::vector<std::unique_ptr<char[]>> files;
        INPathGetFilesInDirectory(dir, recursive, files);
        return files;
#endif
    }

    std::vector<std::unique_ptr<char[]>> PathGetFilesInDirectoryUTF8(
        const char* dir, bool recursive)
    {
#ifdef _WIN32
        std::unique_ptr<nchar[]> nativePth = StringConvertUTF8ToNativePtr(dir);
        return INPathGetFilesInDirectoryUTF8(nativePth.get(), recursive);
#else
        if (!dir) throw std::invalid_argument("dir was null");
        return INPathGetFilesInDirectoryUTF8(dir, recursive);
#endif
    }

    std::vector<std::unique_ptr<nchar[]>> PathGetFilesInDirectory(
        const char* dir, bool recursive)
    {
        std::vector<std::unique_ptr<nchar[]>> files;

#ifdef _WIN32
        std::unique_ptr<nchar[]> nativePth = StringConvertUTF8ToNativePtr(dir);
        INPathGetFilesInDirectory(nativePth.get(), recursive, files);
#else
        if (!dir) throw std::invalid_argument("dir was null");
        INPathGetFilesInDirectory(dir, recursive, files);
#endif

        return files;
    }

    void INPathCreateDirectory(const nchar* path)
    {
        // Return successfully if path is just empty
        if (!*path) return; // TODO: Is this needed on non-Windows platforms too?

#ifdef _WIN32
        BOOL r = CreateDirectoryW(path, nullptr);
        if (!r && GetLastError() != ERROR_ALREADY_EXISTS)
        {
            std::error_code errorCode(GetLastError(), std::system_category());
            throw std::system_error(errorCode);
        }
#else
        if (mkdir(path, S_IRWXU | S_IRWXG | S_IRWXO) && errno != EEXIST)
        {
            // TODO: Use errno to give a more helpful error
            throw std::runtime_error("mkdir failed");
        }
#endif
    }

    void PathCreateDirectory(const char* path)
    {
#ifdef _WIN32
        std::unique_ptr<nchar[]> nativePth = StringConvertUTF8ToNativePtr(path);
        INPathCreateDirectory(nativePth.get());
#else
        if (!path) throw std::invalid_argument("path was null");
        INPathCreateDirectory(path);
#endif
    }

#ifdef _WIN32
    const nchar* PathGetNamePtr(const nchar* path)
    {
        if (!path) return EmptyStringNative;
        return INPathGetNamePtr(path);
    }

    const nchar* PathGetExtPtrName(const nchar* fileName)
    {
        if (!fileName) return EmptyStringNative;
        return INPathGetExtPtrName(fileName);
    }

    const nchar* PathGetExtsPtrName(const nchar* fileName)
    {
        if (!fileName) return EmptyStringNative;
        return INPathGetExtPtrName<nchar, true>(fileName);
    }

    const nchar* PathGetExtPtr(const nchar* filePath)
    {
        // Return extension pointer
        filePath = PathGetNamePtr(filePath);
        return INPathGetExtPtrName(filePath);
    }

    const nchar* PathGetExtsPtr(const nchar* filePath)
    {
        // Return extension pointer
        filePath = PathGetNamePtr(filePath);
        return INPathGetExtPtrName<nchar, true>(filePath);
    }

    std::unique_ptr<nchar[]> PathGetNameNoExtNamePtr(const nchar* fileName)
    {
        if (!fileName) throw std::invalid_argument("fileName was null");
        return INPathGetNameNoExtNamePtr<nchar, false>(fileName);
    }

    std::unique_ptr<nchar[]> PathGetNameNoExtsNamePtr(const nchar* fileName)
    {
        if (!fileName) throw std::invalid_argument("fileName was null");
        return INPathGetNameNoExtNamePtr<nchar, true>(fileName);
    }

    std::unique_ptr<nchar[]> PathGetNameNoExtPtr(const nchar* filePath)
    {
        if (!filePath) throw std::invalid_argument("filePath was null");
        return INPathGetNameNoExtPtr<nchar, false>(filePath);
    }

    std::unique_ptr<nchar[]> PathGetNameNoExtsPtr(const nchar* filePath)
    {
        if (!filePath) throw std::invalid_argument("filePath was null");
        return INPathGetNameNoExtPtr<nchar, true>(filePath);
    }

    std::size_t PathGetStemRangeName(const nchar* fileName, const nchar*& stemEnd)
    {
        if (!fileName) return 0;
        return INPathGetStemRangeName(fileName, stemEnd);
    }

    std::size_t PathGetStemRange(const nchar* path,
        const nchar*& stemStart, const nchar*& stemEnd)
    {
        if (!path) return 0;

        stemStart = path;
        return INPathGetStemRange(stemStart, stemEnd);
    }

    std::unique_ptr<nchar[]> PathGetStemPtr(const nchar* path)
    {
        if (!path) throw std::invalid_argument("path was null");
        return INPathGetStemPtr(path);
    }

    std::unique_ptr<nchar[]> PathGetParentPtr(const nchar* path)
    {
        if (!path) throw std::invalid_argument("path was null");
        return INPathGetParentPtr(path);
    }

    std::unique_ptr<nchar[]> PathCombinePtr(const nchar* path1,
        const nchar* path2)
    {
        if (!path1 || !path2 || (!*path1 && !*path2))
            throw std::invalid_argument("path1 and/or path2 was null or empty");

        return INPathCombinePtr(path1, path2);
    }

    std::unique_ptr<nchar[]> PathCombinePtr(const char* path1,
        const nchar* path2)
    {
        if (!path1 || !path2 || (!*path1 && !*path2))
            throw std::invalid_argument("path1 and/or path2 was null or empty");

        std::unique_ptr<nchar[]> nativePth = StringConvertUTF8ToNativePtr(path1);
        return INPathCombinePtr(nativePth.get(), path2);
    }

    std::unique_ptr<nchar[]> PathCombinePtr(const nchar* path1,
        const char* path2)
    {
        if (!path1 || !path2 || (!*path1 && !*path2))
            throw std::invalid_argument("path1 and/or path2 was null or empty");

        std::unique_ptr<nchar[]> nativePth = StringConvertUTF8ToNativePtr(path2);
        return INPathCombinePtr(path1, nativePth.get());
    }

    std::unique_ptr<nchar[]> PathRemoveExtPtr(const nchar* filePath)
    {
        if (!filePath) throw std::invalid_argument("filePath was null");
        return INPathRemoveExtPtr<nchar, false>(filePath);
    }

    std::unique_ptr<nchar[]> PathRemoveExtsPtr(const nchar* filePath)
    {
        if (!filePath) throw std::invalid_argument("filePath was null");
        return INPathRemoveExtPtr<nchar, true>(filePath);
    }

    bool PathIsDirectory(const nchar* path)
    {
        if (!path) throw std::invalid_argument("path was null");
        return INPathIsDirectory(path);
    }

    bool PathExists(const nchar* path)
    {
        if (!path) throw std::invalid_argument("path was null");
        return INPathExists(path);
    }

    std::size_t PathGetSize(const nchar* filePath)
    {
        if (!filePath) throw std::invalid_argument("filePath was null");
        return INPathGetSize(filePath);
    }

    std::size_t PathGetFileCount(const nchar* dir, bool recursive)
    {
        if (!dir) throw std::invalid_argument("dir was null");
        return INPathGetFileCount(dir, recursive);
    }

    std::vector<std::unique_ptr<char[]>> PathGetFilesInDirectoryUTF8(
        const nchar* dir, bool recursive)
    {
        if (!dir) throw std::invalid_argument("dir was null");
        return INPathGetFilesInDirectoryUTF8(dir, recursive);
    }

    std::vector<std::unique_ptr<nchar[]>> PathGetFilesInDirectory(
        const nchar* dir, bool recursive)
    {
        if (!dir) throw std::invalid_argument("dir was null");

        std::vector<std::unique_ptr<nchar[]>> files;
        INPathGetFilesInDirectory(dir, recursive, files);
        return files;
    }

    void PathCreateDirectory(const nchar* path)
    {
        if (!path) throw std::invalid_argument("path was null");
        INPathCreateDirectory(path);
    }
#endif
}
