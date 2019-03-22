#include "Shaders/IShader.h"
#include <string_view>

namespace HedgeLib::Shaders
{
    IShaderVariant* IShader::Variant(
        const std::string_view variantType) const noexcept
    {
        return Variants()[VariantIndex(Flag(variantType))].get();
    }
}
