#include "brush.h"

#include "rendermanager.h"

namespace GfxPaint {

Brush::Dab::Dab() :
    type{Type::Distance}, metric{0},
    size{16.0f, 16.0f}, fixedRatio{true}, ratio{1.0f},
    angle{0.0f},
    origin{0.0f, 0.0f}, pixelSnapX{PixelSnap::Auto}, pixelSnapY{PixelSnap::Auto},
    hardness{0.0f}, opacity{1.0f},
    buffer()
{
}

Mat4 Brush::Dab::transform() const
{
    Mat4 matrix;
    matrix.rotate(angle);
    matrix.translate(-origin);
    matrix.scale(size / 2.0f);
    return matrix;
}

Brush::Stroke::Stroke() :
    metric{0},
    continuous(false),
    absoluteSpacing{0.0f, 0.0f}, proportionalSpacing{0.25f, 0.25f},
    dabCount{0}
{
}

Brush::Brush() :
    dab(), stroke()
{
}

Brush::Brush(const Brush &other) :
    dab(other.dab), stroke(other.stroke)
{
}

} // namespace GfxPaint
