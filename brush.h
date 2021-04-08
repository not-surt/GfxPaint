#ifndef BRUSH_H
#define BRUSH_H

#include <QMatrix4x4>
#include <QVector2D>
#include "buffer.h"
#include "types.h"

namespace GfxPaint {

enum class Space {
    Object,
    ObjectAspectCorrected,
    World,
    View,
};

enum class PixelSnap {
    Off,
    Centre,
    Edge,
    Both,
    Auto,
};

struct Brush
{
    struct Dab {
        enum class Type {
            Pixel,
            Distance,
            Buffer,
        };

        Dab();
        Dab(const Dab &other);

    public:
        Dab &operator=(const Dab &) = default;
        inline bool operator==(const Dab &rhs) const = default;
        inline bool operator!=(const Dab &rhs) const = default;

        QMatrix4x4 transform() const;

        Space space;
        Type type;
        int metric;
        QVector2D size;
        bool fixedRatio;
        float ratio;
        float angle;
        QVector2D origin;
        PixelSnap pixelSnapX;
        PixelSnap pixelSnapY;
        float hardness;
        float opacity;
        int blendMode;
        int composeMode;
        Buffer buffer;
    };

    struct Stroke {
        Stroke();
        Stroke(const Stroke &other);

        Stroke &operator=(const Stroke &) = default;
        inline bool operator==(const Stroke &rhs) const = default;
        inline bool operator!=(const Stroke &rhs) const = default;

        Space space;
        int metric;
        bool continuous;
        QVector2D absoluteSpacing;
        QVector2D proportionalSpacing;
        int dabCount;
    };

    Brush();
    Brush(const Brush &other);

    Brush &operator=(const Brush &) = default;
    inline bool operator==(const Brush &rhs) const = default;
    inline bool operator!=(const Brush &rhs) const = default;

    Dab dab;
    Stroke stroke;
};

} // namespace GfxPaint

#endif // BRUSH_H
