#include "Shaders/ForcesShader.h"
#include <stdexcept>
#include <cstdint>
#include <cstddef>
#include <cstring>
#include <memory>
#include <vector>
#include <utility>

using namespace HedgeLib::IO;

namespace HedgeLib::Shaders
{
    void HNRegisterVariable::Read(HedgeLib::IO::File& file) noexcept
    {
        file.Read(&Unknown1);
        file.Read(&RegisterIndex);
        file.ReadString(Name);
        file.Align();
    }

    void HNConstantBuffer::Read(HedgeLib::IO::File& file) noexcept
    {
        file.Read(&Unknown1);
        file.Read(&ConstantBufferSize);
        file.Read(&RegisterIndex);
        file.ReadString(Name);
        file.Align();
    }

    void HNConstantBufferVariable::Read(HedgeLib::IO::File& file) noexcept
    {
        file.Read(&Unknown1);
        file.Read(&ConstantBufferIndex);
        file.Read(&DataOffset);
        file.Read(&DataSize);
        file.ReadString(Name);
        file.Align();
    }

    void HNShaderVariant::Read(HedgeLib::IO::File& file) noexcept
    {
        // Read shader bytecode
        file.Read(&BytecodeSize);
        Bytecode = std::make_unique<std::uint8_t[]>(BytecodeSize);
        file.Read(Bytecode.get(), BytecodeSize, 1);
    }

    void HNShader::Read(HedgeLib::IO::File& file)
    {
        auto nodeSize = file.ReadUInt();
        Shader.Read(file);
        InputShader.Read(file);

        // Read Shader Variables
        long pos = file.Tell();
        long variablesEnd = (pos + static_cast<long>(file.ReadUInt()));
        pos += sizeof(std::uint32_t);

        while (pos < variablesEnd)
        {
            // Get variable type
            HNVariableType type = static_cast<HNVariableType>(file.ReadUInt());

            // Read variables of type
            pos += sizeof(std::uint32_t);
            long typeEnd = (pos + static_cast<long>(file.ReadUInt()));

            while (pos < typeEnd)
            {
                switch (type)
                {
                case HN_INPUT_TYPE_TEXTURE:
                    Textures.push_back(HNRegisterVariable());
                    Textures.back().Read(file);
                    break;

                case HN_INPUT_TYPE_SAMPLER:
                    Samplers.push_back(HNRegisterVariable());
                    Samplers.back().Read(file);
                    break;

                case HN_INPUT_TYPE_CONSTANT_BUFFER:
                    ConstantBuffers.push_back(HNConstantBuffer());
                    ConstantBuffers.back().Read(file);
                    break;

                case HN_INPUT_TYPE_BOOLEAN:
                    Bools.push_back(HNConstantBufferVariable());
                    Bools.back().Read(file);
                    break;

                case HN_INPUT_TYPE_INT:
                    Ints.push_back(HNConstantBufferVariable());
                    Ints.back().Read(file);
                    break;

                case HN_INPUT_TYPE_FLOAT:
                    Floats.push_back(HNConstantBufferVariable());
                    Floats.back().Read(file);
                    break;

                default:
                    throw std::logic_error(
                        "Could not read Forces Shader variable; unknown type!");
                }

                pos = file.Tell();
            }
        }
    }

    std::size_t HNShader::BytecodeSize() const noexcept
    {
        return static_cast<std::size_t>(Shader.BytecodeSize);
    }

    std::uint8_t* HNShader::Bytecode() const noexcept
    {
        return Shader.Bytecode.get();
    }

    std::size_t HNShader::SignatureBytecodeSize() const noexcept
    {
        return static_cast<std::size_t>(InputShader.BytecodeSize);
    }

    std::uint8_t* HNShader::SignatureBytecode() const noexcept
    {
        return InputShader.Bytecode.get();
    }

    void ForcesShader::Read(File& file)
    {
        file.BigEndian = true;

        // HHNEEDLE
        DHHNeedleNodeHeader nodeHeader;
        file.Read(&nodeHeader);

        if (nodeHeader.Signature != HHNEEDLESignature)
            throw std::logic_error("Could not read Forces Shader; Invalid HHNEEDLE signature!");

        file.ReadString(BuildPath);
        file.Align();

        // HNSHV002
        file.Read(&nodeHeader);
        if (nodeHeader.Signature != HNSHV002Signature)
            throw std::logic_error("Could not read Forces Shader; Invalid HNSHV002 signature!");

        // Read shader header
        long pos = file.Tell();
        DHNShaderHeader header;
        file.Read(&header);

        long shaderEnd = (pos + static_cast<long>(nodeHeader.Size));

        // Attributes
        attributes.resize(static_cast<std::size_t>(header.AttributeCount));
        for (std::uint32_t i = 0; i < header.AttributeCount; ++i)
        {
            auto& attribute = attributes[i];
            file.Read(reinterpret_cast<std::uint32_t*>(&attribute.Type));
            file.ReadString(attribute.Name);
            file.Align();
        }

        // Indices
        std::uint32_t indexCount = (1U << header.AttributeCount);
        indices.resize(static_cast<std::size_t>(indexCount));

        for (std::uint32_t i = 0; i < indexCount; ++i)
        {
            file.Read(&indices[i]);
        }

        // Shader Variants
        auto variantCount = file.ReadUInt();
        variants.resize(variantCount);

        for (std::uint32_t i = 0; i < variantCount; ++i)
        {
            // Read Shader Bytecode
            variants[i] = std::unique_ptr<IShaderVariant>(new HNShader());
            static_cast<HNShader*>(variants[i].get())->Read(file);
        }
    }

    //void ForcesShader::Write(File& file)
    //{
    //	// TODO
    //}

    std::size_t ForcesShader::VariantCount() const noexcept
    {
        return variants.size();
    }

    const std::unique_ptr<IShaderVariant>* ForcesShader::Variants() const noexcept
    {
        return variants.data();
    }

    std::size_t ForcesShader::Flag(const std::string_view variantType) const noexcept
    {
        for (std::size_t i = 0; i < attributes.size(); ++i)
        {
            if (attributes[i].Name == variantType)
            {
                return (static_cast<std::size_t>(1) << i);
            }
        }

        return 0;
    }

    std::size_t ForcesShader::VariantIndex(const std::size_t flags) const noexcept
    {
        return static_cast<std::size_t>(indices[flags]);
    }
}
