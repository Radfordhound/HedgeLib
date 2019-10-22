#pragma once
#include "../src/gfx/viewport.h"
#include <QThread>

namespace HedgeEdit::UI
{
    class ViewportRenderThread : public QThread
    {
        Q_OBJECT

    private:
        GFX::Viewport* vp;
        volatile bool rendering = false, canRender = true;

    public:
        ViewportRenderThread(GFX::Viewport* vp,
            QObject *parent = nullptr);

        bool IsRendering() const noexcept
        {
            return rendering;
        }

        void StopRendering() noexcept
        {
            canRender = false;
            while (rendering) {} // Block until this render loop stops
        }

        void ResumeRendering() noexcept
        {
            canRender = true;
        }

    public slots:
        void StopRenderingSlot() noexcept
        {
            StopRendering();
        }

    protected:
        void run() override;
    };
}
