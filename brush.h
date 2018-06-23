#ifndef BRUSH_H
#define BRUSH_H

#include <QMatrix4x4>
#include <QVector2D>
#include "buffer.h"
#include "type.h"

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
        inline bool operator==(const Dab &rhs) const {
            return this->space == rhs.space &&
                    this->type == rhs.type &&
                    this->metric == rhs.metric &&
                    this->size == rhs.size &&
                    this->fixedRatio == rhs.fixedRatio &&
                    this->ratio == rhs.ratio &&
                    this->angle == rhs.angle &&
                    this->origin == rhs.origin &&
                    this->pixelSnapX == rhs.pixelSnapX &&
                    this->pixelSnapY == rhs.pixelSnapY &&
                    this->hardness == rhs.hardness &&
                    this->opacity == rhs.opacity &&
                    this->blendMode == rhs.blendMode &&
                    this->composeMode == rhs.composeMode &&
                    this->buffer == rhs.buffer;
        }
        inline bool operator!=(const Dab &rhs) const { return !this->operator==(rhs); }

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

        inline bool operator==(const Stroke &rhs) const {
            return this->space == rhs.space &&
                    this->metric == rhs.metric &&
                    this->continuous == rhs.continuous &&
                    this->absoluteSpacing == rhs.absoluteSpacing &&
                    this->proportionalSpacing == rhs.proportionalSpacing &&
                    this->dabCount == rhs.dabCount;
        }
        inline bool operator!=(const Stroke &rhs) const { return !this->operator==(rhs); }

        Space space;
        int metric;
        bool continuous;
        QVector2D absoluteSpacing;
        QVector2D proportionalSpacing;
        int dabCount;
    };

    Brush();
    Brush(const Brush &other);

    inline bool operator==(const Brush &rhs) const {
        return this->dab == rhs.dab &&
                this->stroke == rhs.stroke;
    }
    inline bool operator!=(const Brush &rhs) const { return !this->operator==(rhs); }

    Dab dab;
    Stroke stroke;
};

} // namespace GfxPaint

#endif // BRUSH_H
