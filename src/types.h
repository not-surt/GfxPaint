#ifndef TYPES_H
#define TYPES_H

#include <QImage>
#include <array>
#include <algorithm>
#include <iterator>
#include <cmath>
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

//typedef QVector2D Vec2;
//typedef QVector3D Vec3;
//typedef QVector4D Vec4;
class Vec2 : public QVector2D {
public:
    using QVector2D::QVector2D;
    Vec2(const Vec2 &other) : QVector2D(static_cast<const QVector2D &>(other)) {}
    Vec2(const QVector2D &other) : QVector2D(other) {}
    Vec2(const float scalar) : Vec2(scalar, scalar) {}
    Vec2& operator=(const Vec2 &other) {
        this->setX(other.x());
        this->setY(other.y());
        return *this;
    }
    Vec2 floor() const { return Vec2(std::floor(this->x()), std::floor(this->y())); }
    Vec2 abs() const { return Vec2(std::abs(this->x()), std::abs(this->y())); }
};
inline QVector2D min(const QVector2D &a, const QVector2D &b) { return QVector2D(std::min(a.x(), b.x()), std::min(a.y(), b.y())); }
inline QVector2D max(const QVector2D &a, const QVector2D &b) { return QVector2D(std::max(a.x(), b.x()), std::max(a.y(), b.y())); }
class Vec3 : public QVector3D {
public:
    using QVector3D::QVector3D;
    Vec3(const Vec3 &other) : QVector3D(static_cast<const QVector3D &>(other)) {}
    Vec3(const QVector3D &other) : QVector3D(other) {}
    Vec3(const float scalar) : Vec3(scalar, scalar, scalar) {}
    Vec3& operator=(const Vec3 &other) {
        this->setX(other.x());
        this->setY(other.y());
        this->setZ(other.z());
        return *this;
    }
    Vec3 floor() const { return Vec3(std::floor(this->x()), std::floor(this->y()), std::floor(this->z())); }
    Vec3 abs() const { return Vec3(std::abs(this->x()), std::abs(this->y()), std::abs(this->z())); }
};
inline QVector3D min(const QVector3D &a, const QVector3D &b) { return QVector3D(std::min(a.x(), b.x()), std::min(a.y(), b.y()), std::min(a.z(), b.z())); }
inline QVector3D max(const QVector3D &a, const QVector3D &b) { return QVector3D(std::max(a.x(), b.x()), std::max(a.y(), b.y()), std::max(a.z(), b.z())); }
class Vec4 : public QVector4D {
public:
    using QVector4D::QVector4D;
    Vec4(const Vec4 &other) : QVector4D(static_cast<const QVector4D &>(other)) {}
    Vec4(const QVector4D &other) : QVector4D(other) {}
    Vec4(const float scalar) : Vec4(scalar, scalar, scalar, scalar) {}
    Vec4& operator=(const Vec4 &other) {
        this->setX(other.x());
        this->setY(other.y());
        this->setZ(other.z());
        this->setW(other.w());
        return *this;
    }
    Vec4 floor() const { return Vec4(std::floor(this->x()), std::floor(this->y()), std::floor(this->z()), std::floor(this->w())); }
    Vec4 abs() const { return Vec4(std::abs(this->x()), std::abs(this->y()), std::abs(this->z()), std::abs(this->w())); }
};
inline QVector4D min(const QVector4D &a, const QVector4D &b) { return QVector4D(std::min(a.x(), b.x()), std::min(a.y(), b.y()), std::min(a.z(), b.z()), std::min(a.w(), b.w())); }
inline QVector4D max(const QVector4D &a, const QVector4D &b) { return QVector4D(std::max(a.x(), b.x()), std::max(a.y(), b.y()), std::max(a.z(), b.z()), std::max(a.w(), b.w())); }

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
    mat4 mat4() const {
        GfxPaint::mat4 mat;
        memcpy(&mat, data(), sizeof(GfxPaint::mat4));
        return mat;
    }
private:
    QPoint map(const QPoint &point) const;
    QPointF map(const QPointF &point) const;
};
inline QVector2D operator*(const QVector2D &vector, const QMatrix4x4 &matrix) { return (vector.toVector3D() * matrix).toVector2D(); }
inline QVector2D operator*(const QMatrix4x4 &matrix, const QVector2D &vector) { return (matrix * vector.toVector3D()).toVector2D(); }
inline Vec2 operator*(const Vec2 &vector, const QMatrix4x4 &matrix) { return Vec2(operator*(static_cast<const QVector2D &>(vector), matrix)); }
inline Vec2 operator*(const QMatrix4x4 &matrix, const Vec2 &vector) { return Vec2(operator*(matrix, static_cast<const QVector2D &>(vector))); }

struct Bounds2 {
    Vec2 min = {std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity()};
    Vec2 max = {-std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity()};

    bool isValid() const {
        return min.x() <= max.x() && min.y() <= max.y();
    }

    Bounds2 expanded(const Bounds2 &bounds) const {
        if (isValid()) return Bounds2{
           {std::min(min.x(), bounds.min.x()), std::min(min.y(), bounds.min.y())},
           {std::max(max.x(), bounds.max.x()), std::max(max.y(), bounds.max.y())},
        };
        else return bounds;
    }
    Bounds2 expanded(const Vec2 &point) const {
        return expanded({point, point});
    }

    Bounds2 clipped(const Bounds2 &other) const {
         return {
            {std::max(min.x(), other.min.x()), std::max(min.y(), other.min.y())},
            {std::min(max.x(), other.max.x()), std::min(max.y(), other.max.y())},
        };
    }

    Bounds2 padded(const Bounds2 &padding) const {
        Q_ASSERT(isValid());
        return {min - padding.min, max + padding.max};
    }

    Bounds2 offset(const Vec2 &offset) const {
        Q_ASSERT(isValid());
        return {min - offset, max - offset};
    }

    static Bounds2 fromCorners(const Vec2 &corner0, const Vec2 &corner1) {
        return {
            {std::min(corner0.x(), corner1.x()), std::min(corner0.y(), corner1.y())},
            {std::max(corner0.x(), corner1.x()), std::max(corner0.y(), corner1.y())},
        };
    }
    static Bounds2 fromCentreAndCorner(const Vec2 &centre, const Vec2 &corner) {
        const Vec2 halfSize = Vec2(corner - centre).abs();
        return {centre - halfSize, centre + halfSize};
    }
    static Bounds2 fromCornerAndSize(const Vec2 &corner, const Vec2 &size) {
        return {corner, corner + size};
    }
    static Bounds2 fromCentreAndSize(const Vec2 &centre, const Vec2 &size) {
        return fromCentreAndCorner(centre, centre + size / 2.0f);
    }
    static Bounds2 fromCentreAndRadius(const Vec2 &centre, const float &radius) {
        return fromCentreAndSize(centre, {radius, radius});
    }
};

struct Bounds3 {
    Vec3 min = {std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity()};
    Vec3 max = {-std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity()};
};

typedef std::list<size_t> GraphIndex;

typedef vec4 Rgba;
typedef GLuint Index;

#define FLOAT_INF std::numeric_limits<GLfloat>::infinity()
#define RGBA_INVALID Rgba{FLOAT_INF, FLOAT_INF, FLOAT_INF, FLOAT_INF}
#define INDEX_INVALID std::numeric_limits<GLuint>::max()

struct alignas(16) Colour {
    alignas(16) Rgba rgba{RGBA_INVALID};
    alignas(4) Index index{INDEX_INVALID};

    //    inline bool operator==(const Colour &rhs) const = default;
    //    inline bool operator!=(const Colour &rhs) const = default;
    inline auto operator<=>(const Colour &rhs) const = default;
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
    Bricks,
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
    std::vector<QString> componentLabels;
    std::vector<std::pair<float, float>> componentRanges;
};

extern std::map<ColourSpace, ColourSpaceInfo> colourSpaceInfo;

} // namespace GfxPaint

#endif // TYPES_H
