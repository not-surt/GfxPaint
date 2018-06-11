#include "brush.h"

#include "rendermanager.h"

namespace GfxPaint {

Dab::Dab() :
    space(Space::Object), type(Type::Distance), metric(0),
    size(16.0, 16.0), fixedRatio(false), ratio(1.0),
    angle(0.0),
    origin(0.0, 0.0), pixelSnapX(PixelSnap::Auto), pixelSnapY(PixelSnap::Auto),
    hardness(0.0), opacity(1.0),
    blendMode(0), composeMode(RenderManager::composeModeDefault),
    buffer()
{
}

Dab::Dab(const Dab &other) :
    space(other.space), type(other.type), metric(other.metric),
    size(other.size), fixedRatio(other.fixedRatio), ratio(other.ratio),
    angle(other.angle),
    origin(other.origin), pixelSnapX(other.pixelSnapX), pixelSnapY(other.pixelSnapY),
    hardness(other.hardness), opacity(other.opacity),
    blendMode(other.blendMode), composeMode(other.composeMode),
    buffer(other.buffer)
{
}

QTransform Dab::transform() const
{
    QTransform transform;
    transform.rotate(angle);
    transform.translate(-origin.x(), -origin.y());
    transform.scale(size.width() / 2.0, size.height() / 2.0);
    return transform;
}

Stroke::Stroke() :
    space(Space::Object), metric(0),
    continuous(false),
    absoluteSpacing(0.0, 0.0), proportionalSpacing(0.25, 0.25),
    dabCount(0)
{
}

Stroke::Stroke(const Stroke &other) :
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
