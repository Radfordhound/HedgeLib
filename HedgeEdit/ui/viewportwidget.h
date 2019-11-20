#pragma once
#include "viewportrenderthread.h"
#include <QWidget>
#include <memory>

namespace HedgeEdit::GFX
{
    class Viewport;
}

namespace HedgeEdit::UI
{
    class ViewportWidget : public QWidget
    {
        Q_OBJECT

    public:
        ViewportWidget(QWidget* parent = nullptr);
        ~ViewportWidget();

    protected:
        std::unique_ptr<GFX::Viewport> vp;
        std::unique_ptr<ViewportRenderThread> renderThread;
        QPoint prevPos;
        bool rotatingCamera = false;

        bool event(QEvent* e) override;
        //void paintEvent(QPaintEvent* e) override;
        void resizeEvent(QResizeEvent* e) override;
        void mousePressEvent(QMouseEvent* e) override;
        void mouseReleaseEvent(QMouseEvent* e) override;
        void mouseMoveEvent(QMouseEvent* e) override;
        void keyPressEvent(QKeyEvent* e) override;
        void keyReleaseEvent(QKeyEvent* e) override;
    };
}
