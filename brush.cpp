#include "brush.h"

namespace GfxPaint {

Dab::Dab() :
    space(Space::Object), type(Type::Distance), metric(Metric::Euclidean),
    size(16.0, 16.0), fixedRatio(false), ratio(1.0),
    angle(0.0),
    origin(0.0, 0.0), pixelSnap(PixelSnap::None),
    hardness(0.0), opacity(1.0),
    blender(Blender::Alpha),
    buffer()
{
}

Dab::Dab(const Dab &other) :
    space(other.space), type(other.type), metric(other.metric),
    size(other.size), fixedRatio(other.fixedRatio), ratio(other.ratio),
    angle(other.angle),
    origin(other.origin), pixelSnap(other.pixelSnap),
    hardness(other.hardness), opacity(other.opacity),
    blender(other.blender),
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
    space(Space::Object),
    continuous(false),
    absoluteSpacing(0.0, 0.0), proportionalSpacing(0.25, 0.25),
    dabCount(0)
{
}

Stroke::Stroke(const Stroke &other) :
    space(other.space),
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
