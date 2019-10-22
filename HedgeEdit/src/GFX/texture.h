#pragma once
#include "HedgeLib/String.h"

#ifdef D3D11
#include "d3d.h"
#include <winrt/base.h>
#include <d3d11.h>
#endif

struct hl_DDSTexture;

namespace HedgeEdit::GFX
{
    class Instance;
    class Texture
    {
#ifdef D3D11
        winrt::com_ptr<ID3D11ShaderResourceView> view;
#endif

    public:
        inline Texture() = default;
#ifdef D3D11
        inline Texture(winrt::com_ptr<ID3D11ShaderResourceView> v) : view(v) {}
#endif
        Texture(const Instance& inst, const hl_DDSTexture* dds, size_t ddsSize);

        void Bind(const Instance& inst, unsigned int slot) const;
    };

#ifdef D3D11
    Texture* LoadDDSTexture(const Instance& inst, const char* filePath);
    Texture* LoadDDSTextureNative(const Instance& inst, const hl_NativeChar* filePath);
#endif
}
