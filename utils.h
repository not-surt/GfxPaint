#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include <QList>
#include <QByteArray>
#include <QWidget>
#include <QKeyEventTransition>
#include <QKeyEvent>
#include <QStateMachine>
#include <QTransform>
#include "opengl.h"
#include "buffer.h"
#include "type.h"

namespace GfxPaint {

QString buildFilterString(QList<QByteArray> imageFormats, QString allLabel = QString("supported"));

QWidget *centringWidget(QWidget *const widget);

// Builtin version doesn't discard key repeats
class KeyEventTransition : public QKeyEventTransition {
public:
    KeyEventTransition(QState *const sourceState = nullptr) : QKeyEventTransition(sourceState) {}
    KeyEventTransition(QObject *object, QEvent::Type type, int key, QState *sourceState = nullptr) : QKeyEventTransition(object, type, key, sourceState) {}

protected:
    virtual bool eventTest(QEvent *event) override {
        const QStateMachine::WrappedEvent *const wrappedEvent = static_cast<const QStateMachine::WrappedEvent *>(event);
        const QKeyEvent *const keyEvent = static_cast<const QKeyEvent *>(wrappedEvent->event());
        return QEventTransition::eventTest(event) &&
                keyEvent->key() == key() &&
                (modifierMask() == Qt::NoModifier || keyEvent->modifiers() & modifierMask()) &&
                !keyEvent->isAutoRepeat();
    }
};

QString fileToString(const QString &path);

void stringMultiReplace(QString &string, const QMap<QString, QString> &replacements);

QTransform viewportTransform(const QSize size);

mat3 qTransformToMat3(const QTransform &transform);
QTransform qTransformFromMat3(const mat3 &matrix);
mat4 qTransformToMat4(const QTransform &transform);
QTransform qTransformFromMat4(const mat4 &matrix);

vec4 qColorToVec4(const QColor &qColor);
QColor qColorFromVec4(const vec4 &colour);

Buffer bufferFromImageFile(const QString &filename, Buffer *const palette = nullptr, Colour *const transparent = nullptr);

template <typename T>
T clamp(const T min, const T max, const T value) {
    return std::max(min, std::min(max, value));
}

float step(const float value, const float size);
float snap(const float offset, const float size, const float target, const bool relative = false, const float relativeTo = 0.0);
QPointF snap2d(const QPointF offset, const QSizeF size, const QPointF target, const bool relative = false, const QPointF relativeTo = QPointF());

void rotateScaleAtOrigin(QTransform &transform, const qreal rotation, const qreal scaling, const QPointF origin);
QTransform transformPointToPoint(const QPointF origin, const QPointF from, const QPointF to);

} // namespace GfxPaint

#endif // UTIL_H
