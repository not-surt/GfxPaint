#include "brush.h"

#include "rendermanager.h"

namespace GfxPaint {

Brush::Dab::Dab() :
    space{Space::Object}, type{Type::Distance}, metric{0},
    size{16.0f, 16.0f}, fixedRatio{false}, ratio{1.0f},
    angle{0.0f},
    origin{0.0f, 0.0f}, pixelSnapX{PixelSnap::Auto}, pixelSnapY{PixelSnap::Auto},
    hardness{0.0f}, opacity{1.0f},
    blendMode{0}, composeMode{RenderManager::composeModeDefault},
    buffer()
{
}

Brush::Dab::Dab(const Dab &other) :
    space(other.space), type(other.type), metric(other.metric),
    size(other.size), fixedRatio(other.fixedRatio), ratio(other.ratio),
    angle(other.angle),
    origin(other.origin), pixelSnapX(other.pixelSnapX), pixelSnapY(other.pixelSnapY),
    hardness(other.hardness), opacity(other.opacity),
    blendMode(other.blendMode), composeMode(other.composeMode),
    buffer(other.buffer)
{
}

QMatrix4x4 Brush::Dab::transform() const
{
    QMatrix4x4 matrix;
    matrix.rotate(angle, {0.0f, 0.0f, 1.0f});
    matrix.translate(-origin.x(), -origin.y());
    matrix.scale(size.x() / 2.0f, size.y() / 2.0f);
    return matrix;
}

Brush::Stroke::Stroke() :
    space{Space::Object}, metric{0},
    continuous(false),
    absoluteSpacing{0.0f, 0.0f}, proportionalSpacing{0.25f, 0.25f},
    dabCount{0}
{
}

Brush::Stroke::Stroke(const Stroke &other) :
    space(other.space), metric(other.metric),
    continuous(other.continuous),
    absoluteSpacing(other.absoluteSpacing), proportionalSpacing(other.proportionalSpacing),
    dabCount(other.dabCount)
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
