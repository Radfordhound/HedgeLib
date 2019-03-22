#include "geometry.h"
#include "device.h"
#include <d3d11.h>
#include <Geometry/ISubMesh.h>
#include <Geometry/VertexFormat.h>
#include <cstdint>
#include <cstddef>
#include <stdexcept>
#include <memory>
#include <algorithm>

using namespace HedgeLib::Geometry;

namespace HedgeEdit::GFX
{
    void Geometry::CreateVertexBufferHH2(Device& device, const ISubMesh* subMesh)
    {
        // Create copy of formats data to operate on
        std::size_t elementCount;
        std::unique_ptr<VertexElement[]> format = subMesh->VertexFormat(elementCount);
        std::unique_ptr<VertexElement[]> elements =
            std::make_unique<VertexElement[]>(elementCount);

        // TODO: Reuse these copies?
        std::copy(format.get(), format.get() + elementCount, elements.get());

        // Compute new Vertex Buffer size
        std::int32_t lastOffset = -1;
        std::uint32_t offset = 0;

        for (std::size_t i = 0; i < elementCount; ++i)
        {
            // Ensure vertex formats are ordered just like the vertex data
            if (static_cast<std::int32_t>(format[i].Offset) <= lastOffset)
            {
                throw std::logic_error(
                    "Vertex Format order does not match Vertex Data!");
            }

            // Compute new offset if offset change is necessary
            lastOffset = static_cast<std::int32_t>(format[i].Offset);
            elements[i].Offset += offset;

            switch (format[i].Format)
            {
            case VERTEX_FORMAT_BYTE4:
            case VERTEX_FORMAT_VECTOR4_BYTE:
                offset += 12;
                break;

            case VERTEX_FORMAT_VECTOR2_HALF:
                offset += 4;
                break;

            case VERTEX_FORMAT_VECTOR3_HH1:
            case VERTEX_FORMAT_VECTOR3_HH2:
                offset += 8;
                break;
            }
        }

        // TODO: Create input layouts *properly*
        vertexFormatIndex = device.CreateInputLayouts(elements.get(),
            elementCount, *device.GetVertexShader(0));

        // Reformat Vertex Buffer to match input layout
        std::size_t vertexCount = subMesh->VertexCount();
        std::size_t origVertexSize = subMesh->VertexSize();
        std::size_t vertexSize = origVertexSize +
            static_cast<std::size_t>(offset);

        std::unique_ptr<std::uint8_t[]> vertices =
            std::make_unique<std::uint8_t[]>(vertexSize * vertexCount);

        for (std::size_t i = 0; i < elementCount; ++i)
        {
            std::uint8_t* origVertexPtr = (subMesh->Vertices() + format[i].Offset);
            std::uint8_t* vertexPtr = (vertices.get() + elements[i].Offset);

            switch (elements[i].Format)
            {
            case VERTEX_FORMAT_BYTE4:
                // Copy indices and convert from 4 bytes to 4 ints
                for (std::size_t i2 = 0; i2 < vertexCount; ++i2)
                {
                    std::uint8_t* origPtr = origVertexPtr;
                    std::int32_t* ptr = reinterpret_cast<std::int32_t*>(vertexPtr);

                    *ptr++ = static_cast<std::int32_t>(*origPtr++); // byte 1
                    *ptr++ = static_cast<std::int32_t>(*origPtr++); // byte 2
                    *ptr++ = static_cast<std::int32_t>(*origPtr++); // byte 3
                    *ptr = static_cast<std::int32_t>(*origPtr);     // byte 4

                    vertexPtr += vertexSize;
                    origVertexPtr += origVertexSize;
                }
                break;

            case VERTEX_FORMAT_VECTOR2:
                // Copy vector
                for (std::size_t i2 = 0; i2 < vertexCount; ++i2)
                {
                    std::uint64_t* origPtr = reinterpret_cast<std::uint64_t*>(origVertexPtr);
                    std::uint64_t* ptr = reinterpret_cast<std::uint64_t*>(vertexPtr);

                    *ptr = static_cast<std::uint64_t>(*origPtr); // X, Y

                    vertexPtr += vertexSize;
                    origVertexPtr += origVertexSize;
                }
                break;

            // TODO: VERTEX_FORMAT_VECTOR2_HALF

            case VERTEX_FORMAT_VECTOR3:
                // Copy vector
                for (std::size_t i2 = 0; i2 < vertexCount; ++i2)
                {
                    float* origPtr = reinterpret_cast<float*>(origVertexPtr);
                    float* ptr = reinterpret_cast<float*>(vertexPtr);

                    *ptr++ = static_cast<float>(*origPtr++); // X
                    *ptr++ = static_cast<float>(*origPtr++); // Y
                    *ptr = static_cast<float>(*origPtr);     // Z

                    vertexPtr += vertexSize;
                    origVertexPtr += origVertexSize;
                }
                break;

            // TODO: VERTEX_FORMAT_VECTOR3_HH1
            // TODO: VERTEX_FORMAT_VECTOR3_HH2

            case VERTEX_FORMAT_VECTOR4:
                // Copy vector
                for (std::size_t i2 = 0; i2 < vertexCount; ++i2)
                {
                    std::uint64_t* origPtr = reinterpret_cast<std::uint64_t*>(origVertexPtr);
                    std::uint64_t* ptr = reinterpret_cast<std::uint64_t*>(vertexPtr);

                    *ptr++ = static_cast<std::uint64_t>(*origPtr++); // X, Y
                    *ptr = static_cast<std::uint64_t>(*origPtr);     // Z, W

                    vertexPtr += vertexSize;
                    origVertexPtr += origVertexSize;
                }
                break;

            case VERTEX_FORMAT_VECTOR4_BYTE:
                // Copy vector and convert from 4 bytes to 4 floats
                for (std::size_t i2 = 0; i2 < vertexCount; ++i2)
                {
                    std::uint8_t* origPtr = origVertexPtr;
                    float* ptr = reinterpret_cast<float*>(vertexPtr);

                    *ptr++ = static_cast<float>(*origPtr++) / 255.0f; // X
                    *ptr++ = static_cast<float>(*origPtr++) / 255.0f; // Y
                    *ptr++ = static_cast<float>(*origPtr++) / 255.0f; // Z
                    *ptr = static_cast<float>(*origPtr) / 255.0f;     // W

                    vertexPtr += vertexSize;
                    origVertexPtr += origVertexSize;
                }
                break;

            default:
                throw std::logic_error("Unsupported vertex format!");
            }
        }

        // Set stride and face count
        stride = static_cast<UINT>(vertexSize);
        faceCount = static_cast<UINT>(subMesh->FaceCount());

        // Create Vertex Buffer
        D3D11_BUFFER_DESC bufferDesc = {};
        bufferDesc.ByteWidth = static_cast<UINT>(
            vertexSize * vertexCount);

        bufferDesc.Usage = D3D11_USAGE_IMMUTABLE;
        bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;

        D3D11_SUBRESOURCE_DATA bufferData = {};
        bufferData.pSysMem = vertices.get();

        HRESULT result = device.D3DDevice()->CreateBuffer(
            &bufferDesc, &bufferData, &vertexBuffer);

        if (FAILED(result))
        {
            throw std::runtime_error(
                "Could not create a Direct3D 11 Vertex Buffer!");
        }

        // Create Index Buffer
        bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
        bufferDesc.ByteWidth = static_cast<UINT>(
            sizeof(std::uint16_t) * faceCount);

        bufferData.pSysMem = subMesh->Faces();

        result = device.D3DDevice()->CreateBuffer(
            &bufferDesc, &bufferData, &indexBuffer);

        if (FAILED(result))
        {
            throw std::runtime_error(
                "Could not create a Direct3D 11 Index Buffer!");
        }
    }

    void Geometry::Create(Device& device, const ISubMesh* subMesh)
    {
        switch (device.RenderMode())
        {
        case RENDER_MODE_HH2:
            CreateVertexBufferHH2(device, subMesh);
            break;

        default:
            // TODO
            break;
        }
    }

    void Geometry::Bind(ID3D11DeviceContext* context) const
    {
        context->IASetVertexBuffers(Device::VertexBufferSlot, 1,
            &vertexBuffer, &stride, &VertexBufferOffset);
        context->IASetIndexBuffer(indexBuffer, DXGI_FORMAT_R16_UINT, 0);
    }

    void Geometry::Draw(ID3D11DeviceContext* context) const
    {
        context->DrawIndexed(faceCount, 0, 0);
    }
}
