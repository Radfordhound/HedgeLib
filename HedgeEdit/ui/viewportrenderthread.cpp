#include "viewportrenderthread.h"
#include <QThread>

namespace HedgeEdit::UI
{
    //ViewportRenderThread::ViewportRenderThread() : QThread() {}

    ViewportRenderThread::ViewportRenderThread(
        GFX::Viewport* vp, QObject *parent) : QThread(parent)
    {
        this->vp = vp;
    }

    void ViewportRenderThread::run()
    {
        if (!vp) return;

        while (!isInterruptionRequested())
        {
            if (!canRender)
                continue;

            // Update
            vp->Update();

            // Render
            rendering = true;
            vp->Render();
            // TODO: Should we delay this or something?
            rendering = false;
        }
    }
}
