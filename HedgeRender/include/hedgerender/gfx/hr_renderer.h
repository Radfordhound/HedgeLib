#ifndef HR_RENDERER_H_INCLUDED
#define HR_RENDERER_H_INCLUDED
#include "hr_gfx_internal.h"

namespace hr
{
namespace gfx
{
class render_device;
class render_graph;

class default_renderer
{
    render_device* m_device;

public:
    HR_GFX_API void render(render_graph& graph);

    HR_GFX_API default_renderer(render_device& device);
};
} // gfx
} // hr
#endif
