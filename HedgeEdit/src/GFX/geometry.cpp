#include "geometry.h"
#include "inputLayout.h"
#include "material.h"
#include "shader.h"
#include "instance.h"
#include <HedgeLib/Geometry/HHSubMesh.h>
#include <stdexcept>
#include <algorithm>
#include <cmath>

namespace HedgeEdit::GFX
{
#ifdef D3D11
    static const UINT vertexBufferOffset = 0;
#endif

    std::int8_t Convert_R10_SINT_To_R8_SNORM(std::uint16_t input)
    {
        float output = static_cast<float>(input & 0x1FF);       // Get 9-bit value
        output += (((input & 0x200) == 0) ? 0.0f : -512.0f);    // Apply sign
        output /= 512.0f;                                       // Get normalized value
        output *= 128.0f;                                       // Denormalize to 8-bit value
        output = std::floor(output);                            // Floor because Forces does it idk
        return static_cast<std::int8_t>(output);                // Cast to signed byte and return
    }

    std::uint32_t Convert_R10G10B10A2_SNORM_To_R8G8B8A8_SNORM(std::uint32_t input)
    {
        std::uint32_t output = static_cast<std::uint32_t>(
            Convert_R10_SINT_To_R8_SNORM(input & 0x3FF));                                   // 10-bit red

        output |= ((Convert_R10_SINT_To_R8_SNORM((input >> 10) &                            // 10-bit green
            0x3FF) << 8) & 0xFF00);

        output |= ((Convert_R10_SINT_To_R8_SNORM((input >> 20) &                            // 10-bit blue
            0x3FF) << 16) & 0xFF0000);

        output |= ((((input & 0x80000000) ? -2 : 0) + ((input >> 30) & 1)) << 24);          // 2-bit alpha
        return output;
    }

#ifdef D3D11
    winrt::com_ptr<ID3D11Buffer> CreateVertexBuffer(
        const Instance& inst, const hl::HHSubMesh& subMesh)
    {
        // Get vertex buffer size
        std::size_t bufSize = (static_cast<std::size_t>(
            subMesh.VertexSize) * subMesh.VertexCount);

        // Create copy of buffer that we can modify before sending to the GPU
        const std::uint8_t* origData = subMesh.Vertices.Get();
        std::unique_ptr<std::uint8_t[]> data = std::unique_ptr<std::uint8_t[]>(
            new std::uint8_t[bufSize]);

        std::copy(origData, origData + bufSize, data.get());

        // Convert data if necessary
        const hl::HHVertexElement* format = subMesh.VertexElements.Get();
        for (; format->Format != hl::HHVERTEX_FORMAT_LAST_ENTRY; ++format)
        {
            switch (format->Format)
            {
            case hl::HHVERTEX_FORMAT_VECTOR3_HH1:
                // ? -> DXGI_FORMAT_R8G8B8A8_SNORM ??
                // TODO
                throw std::runtime_error("HH1 Vector3 unpacking not yet implemented!");

            case hl::HHVERTEX_FORMAT_VECTOR3_HH2:
                // R10G10B10A2_SNORM -> DXGI_FORMAT_R8G8B8A8_SNORM
                std::uint32_t* dst = reinterpret_cast<std::uint32_t*>(
                    data.get() + format->Offset);

                *dst = Convert_R10G10B10A2_SNORM_To_R8G8B8A8_SNORM(
                    *reinterpret_cast<const std::uint32_t*>(origData + format->Offset));
                break;
            }
        }

        // Create a buffer description
        D3D11_BUFFER_DESC desc =
        {
            static_cast<UINT>(bufSize),                     // ByteWidth
            D3D11_USAGE_IMMUTABLE,                          // Usage
            D3D11_BIND_VERTEX_BUFFER,                       // BindFlags
            0,                                              // CPUAccessFlags
            0,                                              // MiscFlags
            0                                               // StructureByteStride
        };

        // Create initial data structure
        D3D11_SUBRESOURCE_DATA initialData = {};
        initialData.pSysMem = data.get();

        // Create vertex buffer
        winrt::com_ptr<ID3D11Buffer> buffer;
        HRESULT result = inst.Device->CreateBuffer(&desc, &initialData, buffer.put());

        // Free data and return buffer
        if (FAILED(result)) throw std::runtime_error("Could not create vertex buffer!");
        return buffer;
    }

    winrt::com_ptr<ID3D11Buffer> CreateIndexBuffer(
        const Instance& inst, const hl::HHSubMesh& subMesh)
    {
        // Create a buffer description
        D3D11_BUFFER_DESC desc =
        {
            sizeof(std::uint16_t) * subMesh.Faces.Count,    // ByteWidth
            D3D11_USAGE_IMMUTABLE,                          // Usage
            D3D11_BIND_INDEX_BUFFER,                        // BindFlags
            0,                                              // CPUAccessFlags
            0,                                              // MiscFlags
            0                                               // StructureByteStride
        };

        // Create initial data structure
        D3D11_SUBRESOURCE_DATA initialData = {};
        initialData.pSysMem = subMesh.Faces.Get();

        // Create index buffer
        winrt::com_ptr<ID3D11Buffer> buffer;
        HRESULT result = inst.Device->CreateBuffer(&desc, &initialData, buffer.put());
        if (FAILED(result)) throw std::runtime_error("Could not create index buffer!");

        return buffer;
    }
#endif

    Geometry::Geometry(Instance& inst, const hl::HHSubMesh& subMesh,
        bool seeThrough) : SeeThrough(seeThrough)
    {
        // TODO: Materials sdjiogjnspigos
        MaterialIndex = 0;

        // Get input layout
        const hl::HHVertexElement* format = subMesh.VertexElements.Get();
        vertexFormatHash = inst.HashVertexFormat(format);
        InputLayout* inputLayout = inst.GetInputLayout(vertexFormatHash);

        if (!inputLayout)
        {
            // TODO: Get vertex shader properly from material rather than hardcoding it
            //VertexShader* vertexShader = inst.GetVertexShader("common_vs");
            //const VertexShaderVariant& variant = vertexShader->GetVariant(0);
            const VertexShaderVariant* variant = inst.GetDefaultVS();

            // Create input layout
            inputLayout = new InputLayout(inst, format,
                variant->GetSignature(), variant->GetSignatureLength());

            inst.AddInputLayout(vertexFormatHash, inputLayout);
        }

        // Create vertex/index buffer
        vertexBuffer = CreateVertexBuffer(inst, subMesh);
        indexBuffer = CreateIndexBuffer(inst, subMesh);

        // Get stride and face count
        stride = subMesh.VertexSize;
        faceCount = subMesh.Faces.Count;
    }

    void Geometry::Bind(const Instance& inst) const
    {
        // Bind geometry
        inst.GetInputLayout(vertexFormatHash)->Use(inst);

#ifdef D3D11
        ID3D11Buffer* b = vertexBuffer.get();
        inst.Context->IASetVertexBuffers(0, 1, &b, &stride, &vertexBufferOffset);
        inst.Context->IASetIndexBuffer(indexBuffer.get(), DXGI_FORMAT_R16_UINT, 0);
#endif

        // Bind material
        const Material* material = inst.GetMaterial(MaterialIndex);
        // TODO: nullptr check
        material->Bind(inst);
    }

    void Geometry::Draw(const Instance& inst) const
    {
#ifdef D3D11
        inst.Context->DrawIndexed(faceCount, 0, 0);
#endif
    }
}
