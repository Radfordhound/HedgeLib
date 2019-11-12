#pragma once
#ifdef D3D11
#include <winrt/base.h>
#include <d3d11.h>
#endif

namespace hl
{
    struct HHVertexElement;
}

namespace HedgeEdit::GFX
{
    class Instance;

    class InputLayout
    {
#ifdef D3D11
        winrt::com_ptr<ID3D11InputLayout> inputLayout;
#endif

    public:
        InputLayout(const Instance& inst, const hl::HHVertexElement* format,
            const void* signature, std::size_t signatureLength);

        void Use(const Instance& inst) const;
    };
}
