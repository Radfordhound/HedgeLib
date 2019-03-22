#pragma once
#include "IShaderVariant.h"
#include "IO/IFileFormat.h"
#include <string_view>
#include <cstddef>
#include <memory>

namespace HedgeLib::Shaders
{
    class IShader : public IO::IFileFormat
    {
    public:
        virtual std::size_t VariantCount() const noexcept = 0;
        virtual const std::unique_ptr<IShaderVariant>* Variants() const noexcept = 0;
        virtual std::size_t Flag(const std::string_view variantType) const noexcept = 0;
        virtual std::size_t VariantIndex(const std::size_t flags) const noexcept = 0;

        virtual IShaderVariant* Variant(
            const std::string_view variantType) const noexcept;

        // TODO: Setters
    };
}
