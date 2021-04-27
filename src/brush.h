#ifndef BRUSH_H
#define BRUSH_H

#include "buffer.h"
#include "types.h"

namespace GfxPaint {

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

        Mat4 transform() const;

        Type type;
        int metric;
        Vec2 size;
        bool fixedRatio;
        float ratio;
        float angle;
        Vec2 origin;
        PixelSnap pixelSnapX;
        PixelSnap pixelSnapY;
        float hardness;
        float opacity;
        Buffer buffer;
    };

    struct Stroke {
        Stroke();
        Stroke(const Stroke &other);

        Stroke &operator=(const Stroke &) = default;
        inline bool operator==(const Stroke &rhs) const = default;
        inline bool operator!=(const Stroke &rhs) const = default;

        int metric;
        bool continuous;
        Vec2 absoluteSpacing;
        Vec2 proportionalSpacing;
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
