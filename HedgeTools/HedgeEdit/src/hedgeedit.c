#include "hedgeedit.h"
#include "resources.h"
#include "rendering.h"
#include "camera.h"
#include "hedgerender/hr_backend.h"
#include "hedgerender/hr_instance.h"
#include "hedgerender/hr_window.h"
#include "hedgerender/hr_renderer.h"
#include "hedgerender/hr_color.h"

HrInstance* Instance = NULL;
HrWindow Window = { 0 };

static HlResult hedgeEditMainLoop(void)
{
    HlResult result;
    while (!Window.shouldClose)
    {
        /* Update window. */
        hrWindowUpdate(&Window);

        /* Update camera position. */
        updateCamera();

        /* Add queued resources to the resource manager if necessary. */
        /* TODO: Actually implement this and then uncomment the following code. */
        /*if (AddQueuedResources)
        {
            result = hrResMgrAddQueuedResources(ResMgr);
            if (HL_FAILED(result)) return result;

            AddQueuedResources = HL_FALSE;
        }*/

        /* Render the scene. */
        result = renderFrame();
        if (HL_FAILED(result)) return result;
    }

    return HL_RESULT_SUCCESS;
}

HlResult hedgeEditRun(void)
{
    HlResult result;

    /* Create backend and instance. */
    {
        HrAdapter* adapter;

        /* Create backend. */
        result = hrBackendInit();
        if (HL_FAILED(result)) return result;

        /* Get default adapter. */
        result = hrBackendGetDefaultAdapter(&adapter);
        if (HL_FAILED(result)) goto failed_instance;

        /* Create instance. */
        result = hrInstanceCreate(adapter, &Instance);

        /* Destroy adapter. */
        hrAdapterDestroy(adapter);

        /* Return failure if instance creation failed. */
        if (HL_FAILED(result)) goto failed_instance;
    }

    /* Create window. */
    result = hrWindowCreate(HL_NTEXT("HedgeEdit - ALPHA 0.1"), 1280, 720,
        HR_WINDOW_DEF_POS, HR_WINDOW_DEF_POS, &Window);

    if (HL_FAILED(result)) goto failed_window;

    /* Create resource manager. */
    result = hrResMgrCreate(Instance, &ResMgr);
    if (HL_FAILED(result)) goto failed_resmgr;

    /* Create renderer. */
    {
        const HrRendererCreateInfo createInfo =
        {
            2,                          /* frameCount */
            1,                          /* maxThreadCount */
            HR_RENDERER_MODE_STANDARD,  /* initialMode */
            ResMgr                      /* initialResMgr */
        };

        result = hrRendererCreateFromWindow(Instance,
            Window.handle, &createInfo, &Renderer);

        if (HL_FAILED(result)) goto failed_renderer;
    }

    /* Initialize camera. */
    initCamera();

    /* Setup resources. */
    result = setupResources();
    if (HL_FAILED(result)) goto end;

#ifdef _WIN32
    /*
       HACK: Render a single black frame before showing the window on
       Windows to prevent the window from flashing white for one frame.
    */
    result = hrRendererRender(Renderer, &HR_COLOR_BLACK, &MainCamera);
    if (HL_FAILED(result)) goto end;
#endif

    /* Show window. */
    result = hrWindowShow(&Window);
    if (HL_FAILED(result)) goto end;

    /* Main loop. */
    result = hedgeEditMainLoop();

    /* Free resources and return. */
end:
    freeResources();
    hrRendererDestroy(Renderer);

failed_renderer:
    hrResMgrDestroy(ResMgr);

failed_resmgr:
    hrWindowDestroy(&Window);

failed_window:
    hrInstanceDestroy(Instance);

failed_instance:
    hrBackendShutdown();
    return result;
}
