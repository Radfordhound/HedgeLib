#include "viewportwidget.h"
#include "../src/GFX/viewport.h"
#include <QEvent>
#include <QThread>
#include <memory>

using namespace HedgeEdit::GFX;

namespace HedgeEdit::UI
{
	ViewportWidget::ViewportWidget(QWidget* parent)
		: QWidget(parent)
	{
		setAttribute(Qt::WA_NativeWindow);

		// Create viewport
#if D3D
		vp = std::unique_ptr<Viewport>(new Viewport(
			reinterpret_cast<HWND>(winId()),
			static_cast<UINT>(width()),
			static_cast<UINT>(height())));
#endif

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
#ifdef D3D
			// Check to ensure this doesn't happen while the constructor is still going
			if (renderThread != nullptr)
			{
				renderThread->StopRendering();
				// TODO: Is this right? Or do we get the new id from the event somehow?
				vp->ChangeHWnd(reinterpret_cast<HWND>(winId()));
				renderThread->ResumeRendering();
			}
#endif
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
}