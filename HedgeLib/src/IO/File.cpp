#include "HedgeLib/IO/File.h"
#include <algorithm>

#ifdef _WIN32
#include <Windows.h>
#endif

namespace hl
{
    constexpr const nchar* INFileGetOpenMode(const FileMode mode)
    {
        switch (mode)
        {
        case FileMode::ReadBinary:
            return HL_NTEXT("rb");
        case FileMode::WriteBinary:
            return HL_NTEXT("wb");
        case FileMode::AppendBinary:
            return HL_NTEXT("ab");
        case FileMode::ReadUpdateBinary:
            return HL_NTEXT("r+b");
        case FileMode::WriteUpdateBinary:
            return HL_NTEXT("w+b");
        case FileMode::AppendUpdateBinary:
            return HL_NTEXT("a+b");
        case FileMode::ReadText:
            return HL_NTEXT("r");
        case FileMode::WriteText:
            return HL_NTEXT("w");
        case FileMode::AppendText:
            return HL_NTEXT("a");
        case FileMode::ReadUpdateText:
            return HL_NTEXT("r+");
        case FileMode::WriteUpdateText:
            return HL_NTEXT("w+");
        case FileMode::AppendUpdateText:
            return HL_NTEXT("a+");
        default:
            return nullptr;
        }
    }

    void File::OpenNoClose(const char* filePath, FileMode mode)
    {
#ifdef _WIN32
        std::unique_ptr<nchar[]> nativePth = StringConvertUTF8ToNativePtr(filePath);
        OpenNoClose(nativePth.get(), mode);
#else
        if (!filePath) throw std::invalid_argument("filePath was null");

        // UTF-8 fopen
        if (!(f = std::fopen(filePath, INFileGetOpenMode(mode))))
        {
            // TODO: Return better error
            std::string errorMsg = "Failed to open the file at the given path (\"";
            errorMsg += filePath;
            errorMsg += "\").";

            throw std::runtime_error(errorMsg);
        }

        isOpen = true;
#endif
    }

#ifdef _WIN32
    void File::OpenNoClose(const nchar* filePath, FileMode mode)
    {
        if (!filePath) throw std::invalid_argument("filePath was null");

        // Windows-specific UTF-16 file open
        if (_wfopen_s(&f, filePath, INFileGetOpenMode(mode)))
        {
            // TODO: Throw better error using GetLastError?
            std::string errorMsg = "Failed to open the file at the given path (\"";
            errorMsg += StringConvertUTF16ToUTF8(reinterpret_cast<const char16_t*>(filePath));
            errorMsg += "\").";

            throw std::runtime_error(errorMsg);
        }

        isOpen = true;
    }
#endif

    void File::Close()
    {
        if (isOpen)
        {
            // Error check
            if (std::fclose(f))
            {
                // TODO: Return a better value than ERROR_UNKNOWN
                throw std::runtime_error("Failed to close file.");
            }

            isOpen = false;
        }
    }

    std::string File::ReadString() const
    {
        std::string str;
        std::uint8_t c;
        
        do
        {
            ReadNoSwap(c);
            if (c == 0) break;
            str += static_cast<char>(c);
        }
        while (true);

        return str;
    }

    std::u16string File::ReadStringUTF16() const
    {
        std::u16string str;
        std::uint16_t c;

        do
        {
            ReadNoSwap(c);
            if (c == 0) break;
            str += static_cast<char16_t>(c);
        }
        while (true);

        return str;
    }

    void File::WriteNulls(std::size_t amount) const
    {
        // Create small (1-16 bytes in size) arrays on
        // the stack instead of the heap for efficiency.
        switch (amount)
        {
        case 0:
            return; // No need to write anything in this case obviously
        case 1:
            WriteNull();
            return;
        case 2:
            INWriteNulls<2>();
            return;
        case 3:
            INWriteNulls<3>();
            return;
        case 4:
            INWriteNulls<4>();
            return;
        case 5:
            INWriteNulls<5>();
            return;
        case 6:
            INWriteNulls<6>();
            return;
        case 7:
            INWriteNulls<7>();
            return;
        case 8:
            INWriteNulls<8>();
            return;
        case 9:
            INWriteNulls<9>();
            return;
        case 10:
            INWriteNulls<10>();
            return;
        case 11:
            INWriteNulls<11>();
            return;
        case 12:
            INWriteNulls<12>();
            return;
        case 13:
            INWriteNulls<13>();
            return;
        case 14:
            INWriteNulls<14>();
            return;
        case 15:
            INWriteNulls<15>();
            return;
        case 16:
            INWriteNulls<16>();
            return;
        }

        // Allocate larger arrays on the heap
        std::unique_ptr<std::uint8_t[]> nulls = std::make_unique<std::uint8_t[]>(amount);

        // Write to file
        WriteBytes(nulls.get(), amount);
    }
}
