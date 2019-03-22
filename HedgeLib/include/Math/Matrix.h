#pragma once
#include "IO/Endian.h"
#include "Reflect.h"
#include <array>

namespace HedgeLib::Math
{
    // TODO: Other sizes of matrices

    struct Matrix4x4
    {
        std::array<float, 16> Elements = {};

        constexpr Matrix4x4() = default;
        constexpr Matrix4x4(std::array<float, 16> elements) :
            Elements(elements) {}

        // TODO: More constructors
        // TODO: Matrix-Math functions

        CUSTOM_ENDIAN_SWAP_TWOWAY
        {
            for (auto& e : Elements)
            {
                IO::Endian::Swap32(e);
            }
        }
    };
}
