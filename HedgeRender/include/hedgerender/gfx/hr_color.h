#ifndef HR_COLOR_H_INCLUDED
#define HR_COLOR_H_INCLUDED
namespace hr
{
namespace gfx
{
class color
{
public:
    float r;
    float g;
    float b;
    float a;

    constexpr color() noexcept :
        r(0.0f),
        g(0.0f),
        b(0.0f),
        a(0.0f) {}

    constexpr color(float r, float g, float b, float a = 1.0f) noexcept :
        r(r),
        g(g),
        b(b),
        a(a) {}
};

namespace colors
{
constexpr color null = { 0.0f, 0.0f, 0.0f, 0.0f };
constexpr color black = { 0.0f, 0.0f, 0.0f, 1.0f };
constexpr color white = { 1.0f, 1.0f, 1.0f, 1.0f };
constexpr color red = { 1.0f, 0.0f, 0.0f, 1.0f };
constexpr color green = { 0.0f, 1.0f, 0.0f, 1.0f };
constexpr color blue = { 0.0f, 0.0f, 1.0f, 1.0f };
} // colors
} // gfx
} // hr
#endif
