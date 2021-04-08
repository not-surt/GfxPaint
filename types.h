#ifndef TYPES_H
#define TYPES_H

#include <QImage>
#include <array>
#include <algorithm>
#include <iterator>

#include "opengl.h"

namespace GfxPaint {

typedef std::array<GLint, 2> ivec2;
typedef std::array<GLint, 3> ivec3;
typedef std::array<GLint, 4> ivec4;

typedef std::array<GLuint, 2> uvec2;
typedef std::array<GLuint, 3> uvec3;
typedef std::array<GLuint, 4> uvec4;

typedef std::array<GLfloat, 2> vec2;
typedef std::array<GLfloat, 3> vec3;
typedef std::array<GLfloat, 4> vec4;

typedef std::array<vec2, 2> mat2;
typedef std::array<vec3, 3> mat3;
typedef std::array<vec4, 4> mat4;

typedef vec4 Rgba;
typedef GLuint Index;

#define FLOAT_INF std::numeric_limits<GLfloat>::infinity()
#define RGBA_INVALID Rgba{FLOAT_INF, FLOAT_INF, FLOAT_INF, FLOAT_INF}
#define INDEX_INVALID std::numeric_limits<GLuint>::max()

struct Colour {
    Rgba rgba{RGBA_INVALID};
    GLuint index{INDEX_INVALID};

    inline bool operator==(const Colour &rhs) const = default;
    inline bool operator!=(const Colour &rhs) const = default;
    inline auto operator<=>(const Colour&) const = default;
};

inline QDebug operator<<(QDebug debug, const Colour &colour)
{
    QDebugStateSaver saver(debug);
    debug.nospace() << "Colour(Rgba(" << colour.rgba[0] << ", " << colour.rgba[1] << ", " << colour.rgba[2] << ", " << colour.rgba[3] << "), Index(" << colour.index << ")";
    return debug;
}

static const Colour COLOUR_INVALID = Colour{RGBA_INVALID, INDEX_INVALID};

enum class AttributelessModel {
    SingleVertex,
    ClipQuad,
    UnitQuad,
};

enum class Pattern {
    Checkers,
};

enum class ColourSpace {
    RGB,
    sRGB,
    XYZ,
    xyY,
    HSV,
    HSL,
    HCY,
};

typedef std::pair<ColourSpace, ColourSpace> ColourSpaceConversion;

struct ColourSpaceInfo {
    QString label;
    QString funcName;
    int componentCount;
    QStringList componentLabels;
    QList<std::pair<qreal, qreal>> componentRanges;
};

extern QMap<ColourSpace, ColourSpaceInfo> colourSpaceInfo;

} // namespace GfxPaint

#endif // TYPES_H