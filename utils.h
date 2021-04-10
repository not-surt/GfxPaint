#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include <QList>
#include <QByteArray>
#include <QWidget>
//#include <QKeyEventTransition>
//#include <QMouseEventTransition>
#include <QKeyEvent>
//#include <QStateMachine>
#include <cmath>
#include "opengl.h"
#include "buffer.h"
#include "types.h"

namespace GfxPaint {

QString buildFilterString(QList<QByteArray> imageFormats, QString allLabel = QString("supported"));

QWidget *centringWidget(QWidget *const widget);
/*
// Builtin version doesn't discard key repeats
class KeyEventTransition : public QKeyEventTransition {
public:
    KeyEventTransition(QState *const sourceState = nullptr) : QKeyEventTransition(sourceState) {}
    KeyEventTransition(QObject *const object, const QEvent::Type type, const int key, QState *const sourceState = nullptr) : QKeyEventTransition(object, type, key, sourceState) {}

protected:
    virtual bool eventTest(QEvent *event) override;
};

class ModifierKeyEventTransition : public QEventTransition {
public:
    ModifierKeyEventTransition(QState *const sourceState = nullptr) : QEventTransition(sourceState) {}
    ModifierKeyEventTransition(QObject *const object, QState *const sourceState = nullptr) : QEventTransition(object, QEvent::None, sourceState) {}

protected:
    virtual bool eventTest(QEvent *event) override;
};

// Builtin version doesn't exactly match modifier mask
class MouseEventTransition : public QMouseEventTransition {
public:
    MouseEventTransition(QState *const sourceState = nullptr) : QMouseEventTransition(sourceState) {}
    MouseEventTransition(QObject *const object, const QEvent::Type type, const Qt::MouseButton button, QState *const sourceState = nullptr) : QMouseEventTransition(object, type, button, sourceState) {}

protected:
    virtual bool eventTest(QEvent *event) override;
};
*/
QString fileToString(const QString &path);

void stringMultiReplace(QString &string, const QMap<QString, QString> &replacements);

QMatrix4x4 viewportTransform(const QSize size);

vec4 qColorToVec4(const QColor &qColor);
QColor qColorFromVec4(const vec4 &colour);

Buffer bufferFromImageFile(const QString &filename, Buffer *const palette = nullptr, Colour *const transparent = nullptr);

template<typename T>
//constexpr T pi = T(T(4) * T(std::atan(T(1))));
const T pi = T(std::acos(T(-1)));

template<typename T>
const T tau = T(2) * pi<T>;

template <typename T, typename S>
T lerp(const T a, const T b, const S t) {
    return a + (b - a) * t;
}

template <typename T, typename S>
T lerpAngular(const T a, const T b, const S t) {
    return (std::fmod(std::fmod(b - a, 2 * pi<T>) + 3 * pi<T>, 2 * pi<T>) - pi<T>) * t;
}

template <typename T>
T clamp(const T min, const T max, const T value) {
    return std::max(min, std::min(max, value));
}

float step(const float value, const float size);
float snap(const float offset, const float size, const float target, const bool relative = false, const float relativeTo = 0.0);
QVector2D snap(const QVector2D offset, const QVector2D size, const QVector2D target, const bool relative = false, const QVector2D relativeTo = {});

void rotateScaleAtOrigin(QMatrix4x4 &transform, const float rotation, const float scaling, const QVector2D origin);
QMatrix4x4 transformPointToPoint(const QVector2D origin, const QVector2D from, const QVector2D to);

} // namespace GfxPaint

#endif // UTIL_H
