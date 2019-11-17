#pragma once
#include <memory>
#include <cstdint>
#include <cstddef>

namespace hl
{
    enum class BlobFormat : std::uint16_t
    {
        Generic = 0,
        HedgehogEngine,
        BINA,
        PACx
    };

    class Blob
    {
        struct INBlobDeleter
        {
            void operator()(void* ptr) const
            {
                // Since we allocate with global operator new for performance
                // we also have to free with global operator delete.
                ::operator delete(ptr);
            }
        };

        /*! @brief The general format of the data contained within the blob (e.g. BINA). */
        BlobFormat format;

        /*! @brief The specific type of the data contained within the blob (e.g. LW .pac). */
        std::uint16_t type;

        /*! @brief The data contained within the blob. */
        std::unique_ptr<void, INBlobDeleter> data;

    public:
        inline Blob() = default;
        inline Blob(std::size_t size, BlobFormat format = BlobFormat::Generic,
            std::uint16_t type = 0) : format(format), type(type)
        {
            // Allocated using global operator new to avoid initialization for performance
            // while avoiding malloc (which cannot be easily overwritten by the user).
            data = std::unique_ptr<void, INBlobDeleter>(
                ::operator new(size));
        }

        constexpr BlobFormat Format() const noexcept
        {
            return format;
        }

        constexpr std::uint16_t Type() const noexcept
        {
            return type;
        }

        template<typename T = std::uint8_t>
        inline const T* RawData() const noexcept
        {
            return static_cast<const T*>(data.get());
        }

        template<typename T = std::uint8_t>
        inline T* RawData() noexcept
        {
            return static_cast<T*>(data.get());
        }

        const void* Data() const;

        template<typename T>
        inline const T* Data() const
        {
            return static_cast<const T*>(Data());
        }

        template<typename T = void>
        inline T* Data()
        {
            return static_cast<T*>(const_cast<void*>(
                const_cast<const Blob*>(this)->Data()));
        }
    };
}
