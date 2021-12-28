#ifndef STROKE_H
#define STROKE_H

#include <QQuaternion>
#include <chrono>

#include "types.h"
#include "utils.h"

namespace GfxPaint {

struct alignas(16) Point {
    alignas(8) Vec2 pos;
    alignas(4) float pressure;
    alignas(16) QQuaternion quaternion;

    Point(const Vec2 pos, const float pressure, const QQuaternion &quaternion) :
        pos(pos), pressure(pressure), quaternion(quaternion)
    {}
};

struct alignas(16) Stroke {
    struct Point : GfxPaint::Point {
        alignas(4) float age;
        alignas(4) float distance;

        Point(const Vec2 pos, const float pressure, const QQuaternion &quaternion, const float age, const float distance) :
            GfxPaint::Point(pos, pressure, quaternion),
            age(age), distance(distance)
        {}
    };

    std::vector<Point> points = {};
    std::chrono::high_resolution_clock::time_point startTime = {};
    float length = 0.0f;

    const Point &add(const Point &point) {
        points.push_back(point);
        return point;
    }
    const Point &add(const Vec2 &pos, const float pressure, const QQuaternion &quaternion) {
        Point point = {pos, pressure, quaternion, {}, 0.0};
        const auto now = std::chrono::high_resolution_clock::now();
        if (points.empty()) {
            startTime = now;
        }
        else {
            point.distance += std::sqrt(Vec2::dotProduct(points.back().pos, pos));
        }
        point.age = std::chrono::duration_cast<std::chrono::duration<float/*, std::milli*/>>(now - startTime).count();
        points.push_back(point);
        length += point.distance;
        return points.back();
    }
    const Point &add(const GfxPaint::Point &point) {
        return add(point.pos, point.pressure, point.quaternion);
    }

    static Point interpolate(const Point &from, const Point &to, const float position) {
        return {lerp(from.pos, to.pos, position),
                lerp(from.pressure, to.pressure, position),
                QQuaternion::slerp(from.quaternion, to.quaternion, position),
                lerp(from.age, to.age, position),
                lerp(from.distance, to.distance, position)};
    }
};

} // namespace GfxPaint

#endif // STROKE_H
