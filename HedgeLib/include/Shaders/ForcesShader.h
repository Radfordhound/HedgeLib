#pragma once
#include "IShader.h"
#include "IShaderVariant.h"
#include "Offsets.h"
#include "Reflect.h"
#include "IO/DataSignature.h"
#include "IO/File.h"
#include <cstdint>
#include <string>
#include <memory>
#include <vector>
#include <string_view>

namespace HedgeLib::Shaders
{
    static constexpr IO::DataSignature64 HHNEEDLESignature = "HHNEEDLE";
    static constexpr IO::DataSignature64 HNSHV002Signature = "HNSHV002";

    enum HNVariableType : std::uint32_t
    {
        HN_INPUT_TYPE_TEXTURE = 3,
        HN_INPUT_TYPE_SAMPLER = 4, // ??
        HN_INPUT_TYPE_CONSTANT_BUFFER = 5,
        HN_INPUT_TYPE_BOOLEAN = 6,
        HN_INPUT_TYPE_INT = 7,
        HN_INPUT_TYPE_FLOAT = 8
    };

    struct HNVariable
    {
        std::uint32_t Unknown1; // Array count??
        std::string Name;

        inline HNVariable() = default;
        virtual inline ~HNVariable() = default;

        virtual void Read(HedgeLib::IO::File& file) noexcept = 0;
        //virtual void Write(HedgeLib::IO::File& file) const noexcept = 0;
    };

    struct HNRegisterVariable : HNVariable
    {
        std::uint32_t RegisterIndex;

        inline HNRegisterVariable() = default;
        inline ~HNRegisterVariable() override = default;

        void Read(HedgeLib::IO::File& file) noexcept override;
        //void Write(HedgeLib::IO::File& file) const noexcept override;
    };

    struct HNConstantBuffer : HNVariable
    {
        std::uint32_t ConstantBufferSize;
        std::uint32_t RegisterIndex;

        inline HNConstantBuffer() = default;
        inline ~HNConstantBuffer() override = default;

        void Read(HedgeLib::IO::File& file) noexcept override;
        //void Write(HedgeLib::IO::File& file) const noexcept override;
    };

    struct HNConstantBufferVariable : HNVariable
    {
        std::uint32_t ConstantBufferIndex;
        std::uint32_t DataOffset;
        std::uint32_t DataSize;

        inline HNConstantBufferVariable() = default;
        inline ~HNConstantBufferVariable() override = default;

        void Read(HedgeLib::IO::File& file) noexcept override;
        //void Write(HedgeLib::IO::File& file) const noexcept override;
    };

    struct HNShaderVariant
    {
        std::uint32_t BytecodeSize = 0;
        std::unique_ptr<std::uint8_t[]> Bytecode = nullptr;

        inline HNShaderVariant() = default;
        /*inline HNShaderVariant(const HNShaderVariant& v) :
            BytecodeSize(v.BytecodeSize)
        {
            Bytecode = std::make_unique<std::uint8_t[]>(BytecodeSize);
            std::memcpy(Bytecode.get(), v.Bytecode.get(), BytecodeSize);
        }

        inline HNShaderVariant(HNShaderVariant&& v) :
            BytecodeSize(std::move(v.BytecodeSize)),
            Bytecode(std::move(v.Bytecode)) {}*/

        void Read(HedgeLib::IO::File& file) noexcept;
        //void Write(HedgeLib::IO::File& file) const noexcept;
    };

    class HNShader : public IShaderVariant
    {
    public:
        HNShaderVariant Shader;
        HNShaderVariant InputShader;

        std::vector<HNRegisterVariable> Textures;
        std::vector<HNRegisterVariable> Samplers;
        std::vector<HNConstantBuffer> ConstantBuffers;
        std::vector<HNConstantBufferVariable> Bools;
        std::vector<HNConstantBufferVariable> Ints;
        std::vector<HNConstantBufferVariable> Floats;

        inline HNShader() = default;
        //inline HNShader(const HNShader& v) : Shader(v.Shader),
        //    InputShader(v.InputShader), Variables(v.Variables)
        //{
        //    /*Bytecode = std::make_unique<std::uint8_t[]>(BytecodeSize);
        //    std::memcpy(Bytecode.get(), v.Bytecode.get(), BytecodeSize);*/
        //}

        //inline HNShader(HNShader&& v) :
        //    Shader(std::move(v.Shader)),
        //    InputShader(std::move(v.InputShader)),
        //    Variables(std::move(Variables)) {}

        inline ~HNShader() override = default;

        void Read(HedgeLib::IO::File& file);
        //void Write(HedgeLib::IO::File& file) const noexcept;

        std::size_t BytecodeSize() const noexcept override;
        std::uint8_t* Bytecode() const noexcept override;

        std::size_t SignatureBytecodeSize() const noexcept override;
        std::uint8_t* SignatureBytecode() const noexcept override;
    };

    enum HNShaderAttributeType : std::uint32_t
    {
        HN_SHADER_ATTRIB_TYPE_UNKNOWN = 0xFFFFFFFF,
        HN_SHADER_ATTRIB_TYPE_ATTRIBUTE = 0,
        HN_SHADER_ATTRIB_TYPE_TEXTURE = 1
    };

    struct HNShaderAttribute
    {
        HNShaderAttributeType Type;
        std::string Name;
    };

    struct DHNShaderAttribute
    {
        HNShaderAttributeType Type;
        char Name; // Do &Name to get the null-terminated string
    };

    struct DHNShaderHeader
    {
        std::uint32_t Unknown1; // Always 0??
        std::uint32_t AttributeCount;

        ENDIAN_SWAP(Unknown1, AttributeCount);
    };

    struct DHHNeedleNodeHeader
    {
        IO::DataSignature64 Signature;
        std::uint32_t Size;

        ENDIAN_SWAP(Size);
    };

    struct DHHNeedleNode
    {
        // Do &BuildPath to get the null-terminated string
        char BuildPath;
    };

    class ForcesShader : public IShader
    {
        std::vector<HNShaderAttribute> attributes;
        std::vector<std::uint32_t> indices;
        std::vector<std::unique_ptr<IShaderVariant>> variants;

    public:
        std::string BuildPath;

        inline ForcesShader() = default;
        inline ~ForcesShader() override = default;

        void Read(HedgeLib::IO::File& file) override;
        //void Write(HedgeLib::IO::File& file) override;

        std::size_t VariantCount() const noexcept override;
        const std::unique_ptr<IShaderVariant>* Variants() const noexcept override;
        std::size_t Flag(const std::string_view variantType) const noexcept override;
        std::size_t VariantIndex(const std::size_t flags) const noexcept override;
    };
}
