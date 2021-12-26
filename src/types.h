#ifndef TYPES_H
#define TYPES_H

#include <QImage>
#include <array>
#include <algorithm>
#include <iterator>
#include <QVector2D>
#include <QVector3D>
#include <QMatrix4x4>

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
    Rgba rgba alignas(16){RGBA_INVALID};
    GLuint index alignas(4){INDEX_INVALID};

    inline bool operator==(const Colour &rhs) const = default;
    inline bool operator!=(const Colour &rhs) const = default;
    inline auto operator<=>(const Colour&) const = default;
};

typedef QVector2D Vec2;
typedef QVector3D Vec3;
typedef QVector4D Vec4;

typedef std::list<size_t> GraphIndex;

class Mat4 : public QMatrix4x4 {
public:
    using QMatrix4x4::QMatrix4x4;
    Mat4(const QMatrix4x4 &other) : QMatrix4x4(other) {}
    Mat4 inverted(bool *invertible = nullptr) const { return QMatrix4x4::inverted(invertible); }
    using QMatrix4x4::map;
    QVector2D map(const QVector2D &point) const { return QMatrix4x4::map(point.toVector3D()).toVector2D(); }
    using QMatrix4x4::mapVector;
    QVector2D mapVector(const QVector2D &point) const { return QMatrix4x4::mapVector(point.toVector3D()).toVector2D(); }
    using QMatrix4x4::rotate;
    void rotate(const float angle) { QMatrix4x4::rotate(angle, QVector3D(0.0f, 0.0f, -1.0f)); }
    using QMatrix4x4::scale;
    void scale(const QVector2D &vector) { QMatrix4x4::scale(vector.x(), vector.y(), 1.0f); }
    using QMatrix4x4::translate;
    void translate(const QVector2D &vector) { QMatrix4x4::translate(vector.toVector3D()); }
private:
    QPoint map(const QPoint &point) const;
    QPointF map(const QPointF &point) const;
};
inline QVector2D operator*(const QVector2D &vector, const QMatrix4x4 &matrix) { return (vector.toVector3D() * matrix).toVector2D(); }
inline QVector2D operator*(const QMatrix4x4 &matrix, const QVector2D &vector) { return (matrix * vector.toVector3D()).toVector2D(); }

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
