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
            Distance,
            Buffer,
        };

        Dab();
        Dab(const Dab &other) = default;

    public:
        Dab &operator=(const Dab &) = default;
        inline bool operator==(const Dab &rhs) const = default;
        inline bool operator!=(const Dab &rhs) const = default;

        Mat4 transform() const;

        alignas(4) Type type;
        alignas(4) int metric;
        alignas(8) Vec2 size;
        alignas(4) bool fixedRatio;
        alignas(4) float ratio;
        alignas(4) float angle;
        alignas(8) Vec2 origin;
        alignas(4) PixelSnap pixelSnapX;
        alignas(4) PixelSnap pixelSnapY;
        alignas(4) float hardness;
        alignas(4) float opacity;
        Buffer buffer;
    };

    struct Stroke {
        Stroke();
        Stroke(const Stroke &other) = default;

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
