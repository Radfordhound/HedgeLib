#include "constantBuffer.h"
#include "instance.h"
#include <stdexcept>
#include <algorithm>

namespace HedgeEdit::GFX
{
#ifdef D3D11
    ConstantBuffer::ConstantBuffer(const Instance& inst, std::size_t bufferSize,
        ska::flat_hash_map<std::string, ConstantBufferVariable> variables,
        const D3D11_USAGE usage, const D3D11_CPU_ACCESS_FLAG flags,
        const void* initialData) : variables(variables)
    {
        // Initialize subresource pointer
        mappedRes.pData = nullptr;

        // Create Constant Buffer
        D3D11_BUFFER_DESC bufferDesc = {};
        bufferDesc.ByteWidth = static_cast<UINT>(bufferSize);
        bufferDesc.Usage = usage;
        bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
        bufferDesc.CPUAccessFlags = flags;

        HRESULT result;
        if (initialData)
        {
            D3D11_SUBRESOURCE_DATA bufferData = {};
            bufferData.pSysMem = initialData;

            result = inst.Device->CreateBuffer(
                &bufferDesc, &bufferData, buffer.put());
        }
        else
        {
            result = inst.Device->CreateBuffer(
                &bufferDesc, nullptr, buffer.put());
        }

        // Check for errors
        if (FAILED(result))
        {
            throw std::runtime_error(
                "Could not create a Direct3D 11 Constant Buffer!");
        }
    }

    void ConstantBuffer::Map(ID3D11DeviceContext* context, D3D11_MAP mapMode)
    {
        HRESULT result = context->Map(buffer.get(), 0, mapMode, 0, &mappedRes);
        if (FAILED(result))
        {
            throw std::runtime_error(
                "Could not map Direct3D 11 Constant Buffer!");
        }
    }

    void ConstantBuffer::Unmap(ID3D11DeviceContext* context)
    {
        context->Unmap(buffer.get(), 0);
        mappedRes.pData = nullptr;
    }
#endif

    void ConstantBuffer::SetValue(const std::string& name, const void* value)
    {
        // Return if not mapped
        if (!mappedRes.pData) return;

        // Get pointer to variable within constant buffer
        auto it = variables.find(name);
        if (it == variables.end()) return;

        ConstantBufferVariable v = it->second;
        std::uint8_t* dst = (static_cast<std::uint8_t*>(
            mappedRes.pData) + v.DataOffset);

        // Copy the data from value to the constant buffer
        const std::uint8_t* src = static_cast<const std::uint8_t*>(value);
        std::copy(src, src + v.DataSize, dst);
    }

    void ConstantBuffer::UseVS(const Instance& inst, unsigned int slot) const
    {
        ID3D11Buffer* b = buffer.get();
        inst.Context->VSSetConstantBuffers(slot, 1, &b);
    }

    void ConstantBuffer::UsePS(const Instance& inst, unsigned int slot) const
    {
        ID3D11Buffer* b = buffer.get();
        inst.Context->PSSetConstantBuffers(slot, 1, &b);
    }
}
