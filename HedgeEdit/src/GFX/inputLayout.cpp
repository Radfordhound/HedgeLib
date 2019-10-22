#include "inputLayout.h"
#include "instance.h"
#include <HedgeLib/Geometry/HHSubMesh.h>
#include <stdexcept>
#include <array>

namespace HedgeEdit::GFX
{
#ifdef D3D11
    static const std::array<D3D11_INPUT_ELEMENT_DESC, 4> StandardInputElements =
    {{
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT }
    }};

    static const std::array<D3D11_INPUT_ELEMENT_DESC, 16> HH2InputElements =
    {{
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT },
        { "BLENDWEIGHT", 0, DXGI_FORMAT_R32G32B32_FLOAT },
        { "BLENDWEIGHT", 1, DXGI_FORMAT_R32G32B32_FLOAT },
        { "BLENDINDICES", 0, DXGI_FORMAT_R32G32B32_FLOAT },
        { "BLENDINDICES", 1, DXGI_FORMAT_R32G32B32_FLOAT },
        { "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT },
        { "NORMAL", 1, DXGI_FORMAT_R32G32B32_FLOAT },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32B32_FLOAT },
        { "TEXCOORD", 1, DXGI_FORMAT_R32G32B32_FLOAT },
        { "TEXCOORD", 2, DXGI_FORMAT_R32G32B32_FLOAT },
        { "TEXCOORD", 3, DXGI_FORMAT_R32G32B32_FLOAT },
        { "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT },
        { "TANGENT", 1, DXGI_FORMAT_R32G32B32_FLOAT },
        { "BINORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT },
        { "BINORMAL", 1, DXGI_FORMAT_R32G32B32_FLOAT },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32_FLOAT }
    }};

    DXGI_FORMAT ConvertVertexFormatHH2(const uint32_t format)
    {
        switch (format)
        {
        case HL_HHVERTEX_FORMAT_INDEX_BYTE:
            // TODO: This format is correct, but apparently the game does something weird with the data itself?
            // For example 00 01 02 02 consistently becomes 00 00 5b 42 in memory.
            // Not sure why this is or what these in-memory values actually mean.
            // Also not sure if the Forces shaders do this or the model serializer?
            // If it's the latter, we have to convert these ourselves.
            // Does Gens do this too??
            return DXGI_FORMAT_R8G8B8A8_UINT;

        case HL_HHVERTEX_FORMAT_INDEX:
            // TODO: Is this correct?
            return DXGI_FORMAT_R8G8B8A8_SINT;

        case HL_HHVERTEX_FORMAT_VECTOR2:
            return DXGI_FORMAT_R32G32_FLOAT;

        case HL_HHVERTEX_FORMAT_VECTOR2_HALF:
            return DXGI_FORMAT_R16G16_FLOAT;

        case HL_HHVERTEX_FORMAT_VECTOR3:
            return DXGI_FORMAT_R32G32B32_FLOAT;

        case HL_HHVERTEX_FORMAT_VECTOR3_HH1:
            // TODO
            return DXGI_FORMAT_R8G8B8A8_SNORM;

        case HL_HHVERTEX_FORMAT_VECTOR3_HH2:
            return DXGI_FORMAT_R8G8B8A8_SNORM;

        case HL_HHVERTEX_FORMAT_VECTOR4:
            return DXGI_FORMAT_R32G32B32A32_FLOAT;

        case HL_HHVERTEX_FORMAT_VECTOR4_BYTE:
            return DXGI_FORMAT_R8G8B8A8_UNORM;

        default:
            throw std::runtime_error("Unknown vertex element format!");
        }
    }

    winrt::com_ptr<ID3D11InputLayout> CreateInputLayoutStandard(const Instance& inst,
        const hl_HHVertexElement* format, const void* signature,
        std::size_t signatureLength)
    {
        // Get input elements
        std::array<D3D11_INPUT_ELEMENT_DESC, 4> elements = StandardInputElements;
        size_t i;

        for (; format->Format != HL_HHVERTEX_FORMAT_LAST_ENTRY; ++format)
        {
            // Get corresponding input element, if any. Skip otherwise
            switch (format->Type)
            {
            case HL_HHVERTEX_TYPE_POSITION:
                if (format->Index != 0) continue;
                i = 0;
                break;

            case HL_HHVERTEX_TYPE_NORMAL:
                if (format->Index != 0) continue;
                i = 1;
                break;

            case HL_HHVERTEX_TYPE_COLOR:
                if (format->Index != 0) continue;
                i = 2;
                break;

            case HL_HHVERTEX_TYPE_UV:
                if (format->Index != 0) continue;
                i = 3;
                break;

            default: continue;
            }

            // Set input element format
            elements[i].Format = ConvertVertexFormatHH2(format->Format);

            // Set input element byte offset
            elements[i].AlignedByteOffset = static_cast<UINT>(format->Offset);
        }

        // Create input layout
        winrt::com_ptr<ID3D11InputLayout> inputLayout;
        HRESULT result = inst.Device->CreateInputLayout(elements.data(),
            static_cast<UINT>(elements.size()), signature,
            static_cast<SIZE_T>(signatureLength), inputLayout.put());

        if (FAILED(result))
        {
            // TODO: Maybe give more helpful errors based on result?
            throw std::runtime_error("Failed to create input layout!");
        }

        return inputLayout;
    }

    winrt::com_ptr<ID3D11InputLayout> CreateInputLayoutHH2(const Instance& inst,
        const hl_HHVertexElement* format, const void* signature,
        std::size_t signatureLength)
    {
        // Get input elements
        std::array<D3D11_INPUT_ELEMENT_DESC, 16> elements = HH2InputElements;
        size_t i;

        for (; format->Format != HL_HHVERTEX_FORMAT_LAST_ENTRY; ++format)
        {
            // Get corresponding input element, if any. Skip otherwise
            switch (format->Type)
            {
            case HL_HHVERTEX_TYPE_POSITION:
                if (format->Index != 0) continue;
                i = 0;
                break;

            case HL_HHVERTEX_TYPE_BONE_WEIGHT:
                if (format->Index > 1) continue;
                i = (1 + static_cast<size_t>(format->Index));
                break;

            case HL_HHVERTEX_TYPE_BONE_INDEX:
                if (format->Index > 1) continue;
                i = (3 + static_cast<size_t>(format->Index));
                break;

            case HL_HHVERTEX_TYPE_NORMAL:
                if (format->Index > 1) continue;
                i = (5 + static_cast<size_t>(format->Index));
                break;

            case HL_HHVERTEX_TYPE_UV:
                if (format->Index > 3) continue;
                i = (7 + static_cast<size_t>(format->Index));
                break;

            case HL_HHVERTEX_TYPE_TANGENT:
                if (format->Index > 1) continue;
                i = (11 + static_cast<size_t>(format->Index));
                break;

            case HL_HHVERTEX_TYPE_BINORMAL:
                if (format->Index > 1) continue;
                i = (13 + static_cast<size_t>(format->Index));
                break;

            case HL_HHVERTEX_TYPE_COLOR:
                if (format->Index != 0) continue;
                i = 15;
                break;

            default: continue;
            }

            // Set input element format
            elements[i].Format = ConvertVertexFormatHH2(format->Format);

            // Set input element byte offset
            elements[i].AlignedByteOffset = static_cast<UINT>(format->Offset);
        }

        // Create input layout
        winrt::com_ptr<ID3D11InputLayout> inputLayout;
        HRESULT result = inst.Device->CreateInputLayout(elements.data(),
            static_cast<UINT>(elements.size()), signature,
            static_cast<SIZE_T>(signatureLength), inputLayout.put());

        if (FAILED(result))
        {
            // TODO: Maybe give more helpful errors based on result?
            throw std::runtime_error("Failed to create input layout!");
        }

        return inputLayout;
    }

    InputLayout::InputLayout(const Instance& inst, const hl_HHVertexElement* format,
        const void* signature, std::size_t signatureLength)
    {
        switch (inst.RenderType())
        {
        case RenderTypes::Standard:
            inputLayout = CreateInputLayoutStandard(inst,
                format, signature, signatureLength);
            break;

        case RenderTypes::HedgehogEngine2:
            inputLayout = CreateInputLayoutStandard(inst,
                format, signature, signatureLength);
            break;

        default:
            throw std::runtime_error("Unknown renderer!");
        }
    }

    void InputLayout::Use(const Instance& inst) const
    {
        inst.Context->IASetInputLayout(inputLayout.get());
    }
#endif
}
