#pragma once

#include "../src/GFX/viewport.h"
#include "viewportrenderthread.h"
#include <QWidget>
#include <memory>

namespace HedgeEdit::UI
{
    class ViewportWidget : public QWidget
    {
        Q_OBJECT

    public:
        ViewportWidget(QWidget* parent = nullptr);
        ~ViewportWidget();

    protected:
        std::unique_ptr<HedgeEdit::GFX::Viewport> vp;
        std::unique_ptr<ViewportRenderThread> renderThread;

        bool event(QEvent* e) override;
        //void paintEvent(QPaintEvent* e) override;
        void resizeEvent(QResizeEvent* e) override;
    };
}
