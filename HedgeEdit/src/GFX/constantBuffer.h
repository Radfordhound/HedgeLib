#pragma once

#ifdef D3D11
#include <winrt/base.h>
#include <d3d11.h>
#endif

#include "../flat_hash_map.h"
#include <string>
#include <cstdint>

namespace HedgeEdit::GFX
{
    class Instance;

    struct ConstantBufferVariable
    {
        std::uint32_t DataOffset;
        std::uint32_t DataSize;
    };

    class ConstantBuffer
    {
#ifdef D3D11
        winrt::com_ptr<ID3D11Buffer> buffer;
        D3D11_MAPPED_SUBRESOURCE mappedRes;
#endif

        ska::flat_hash_map<std::string, ConstantBufferVariable> variables;

    public:
#ifdef D3D11
        ConstantBuffer(const Instance& inst, std::size_t bufferSize,
            ska::flat_hash_map<std::string, ConstantBufferVariable> variables,
            const D3D11_USAGE usage = D3D11_USAGE_DEFAULT,
            const D3D11_CPU_ACCESS_FLAG flags = static_cast<D3D11_CPU_ACCESS_FLAG>(0),
            const void* initialData = nullptr);

        void Map(ID3D11DeviceContext* context, D3D11_MAP mapMode);
        void Unmap(ID3D11DeviceContext* context);
#endif
        
        void SetValue(const std::string& name, const void* value);
        void UseVS(const Instance& inst, unsigned int slot) const;
        void UsePS(const Instance& inst, unsigned int slot) const;
    };
}
