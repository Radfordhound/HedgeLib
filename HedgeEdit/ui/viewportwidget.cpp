#include "viewportwidget.h"
#include "../src/gfx/instance.h"
#include "../src/gfx/viewport.h"
#include <QEvent>
#include <QThread>
#include <QMouseEvent>
#include <QApplication>
#include <memory>

using namespace HedgeEdit::GFX;

namespace HedgeEdit::UI
{
    ViewportWidget::ViewportWidget(QWidget* parent)
        : QWidget(parent)
    {
        setAttribute(Qt::WA_NativeWindow);

        // Create viewport
        vp = std::unique_ptr<Viewport>(new Viewport(
            GFX::MainInstance,
            reinterpret_cast<HWND>(winId()),
            static_cast<UINT>(width()),
            static_cast<UINT>(height())));

        // Create render thread
        renderThread = std::unique_ptr<ViewportRenderThread>(
            new ViewportRenderThread(vp.get(), this));

        // Pauses rendering and blocks until it's done when widget is first destroyed
        connect(renderThread.get(), SIGNAL(finished()),
            renderThread.get(), SLOT(StopRenderingSlot()));

        renderThread->start();
    }

    ViewportWidget::~ViewportWidget()
    {
        // Executes after finished() -> pauseRendering() connection setup earlier
        renderThread->requestInterruption();
        renderThread->wait(); // Blocks until thread is done
    }

    bool ViewportWidget::event(QEvent* e)
    {
        if (e->type() == QEvent::WinIdChange)
        {
            // Check to ensure this doesn't happen while the constructor is still going
            if (renderThread != nullptr)
            {
                renderThread->StopRendering();
                // TODO: Is this right? Or do we get the new id from the event somehow?
                vp->ChangeHandle(reinterpret_cast<HWND>(winId()));
                renderThread->ResumeRendering();
            }
        }

        return QWidget::event(e);
    }

    /*void ViewportWidget::paintEvent(QPaintEvent* e)
    {
        vp->Render();
    }*/

    void ViewportWidget::resizeEvent(QResizeEvent* e)
    {
        renderThread->StopRendering();
        vp->Resize(static_cast<FLOAT>(width()),
            static_cast<FLOAT>(height()));

        renderThread->ResumeRendering();
    }

    void ViewportWidget::mousePressEvent(QMouseEvent* e)
    {
        if (e->button() == Qt::RightButton)
        {
            prevPos = e->pos();
            rotatingCamera = true;
            QApplication::setOverrideCursor(Qt::BlankCursor);
        }
    }

    void ViewportWidget::mouseReleaseEvent(QMouseEvent* e)
    {
        if (e->button() == Qt::RightButton)
        {
            rotatingCamera = false;
            QApplication::restoreOverrideCursor();
        }
    }

    void ViewportWidget::mouseMoveEvent(QMouseEvent* e)
    {
        if (rotatingCamera)
        {
            // Rotate Camera
            QPoint curPos = e->pos();
            QPoint dif = (curPos - prevPos);
            
            vp->RotateCamera(dif.x(), -dif.y());

            // Lock cursor to center of viewport if necessary
            if (!geometry().contains(curPos, true))
            {
                curPos = geometry().center();
                QCursor::setPos(mapToGlobal(curPos));
            }

            prevPos = curPos;
        }
    }

    void ViewportWidget::keyPressEvent(QKeyEvent* e)
    {
        if (e->key() == Qt::Key_W) vp->MovingForward = true;
        if (e->key() == Qt::Key_S) vp->MovingBackward = true;
        if (e->key() == Qt::Key_A) vp->MovingLeft = true;
        if (e->key() == Qt::Key_D) vp->MovingRight = true;

        vp->Moving = (vp->MovingForward || vp->MovingBackward ||
            vp->MovingLeft || vp->MovingRight);
    }

    void ViewportWidget::keyReleaseEvent(QKeyEvent* e)
    {
        if (e->key() == Qt::Key_W) vp->MovingForward = false;
        if (e->key() == Qt::Key_S) vp->MovingBackward = false;
        if (e->key() == Qt::Key_A) vp->MovingLeft = false;
        if (e->key() == Qt::Key_D) vp->MovingRight = false;

        vp->Moving = (vp->MovingForward || vp->MovingBackward ||
            vp->MovingLeft || vp->MovingRight);
    }
}
