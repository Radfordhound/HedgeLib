#pragma once
#include <cstddef>
#include <cstdint>

namespace HedgeLib::Shaders
{
    class IShaderVariant
    {
    public:
        virtual ~IShaderVariant() = 0;

        virtual std::size_t BytecodeSize() const noexcept = 0;
        virtual std::uint8_t* Bytecode() const noexcept = 0;

        virtual std::size_t SignatureBytecodeSize() const noexcept = 0;
        virtual std::uint8_t* SignatureBytecode() const noexcept = 0;

        // TODO: Shader Variables
        // TODO: Setters
    };
}
