#ifndef UTILS_H
#define UTILS_H

#include <QString>
#include <QList>
#include <QByteArray>
#include <QWidget>
#include <QEventTransition>
#include <QKeyEvent>
#include <QStateMachine>
#include <QTransform>
#include "opengl.h"
#include "buffer.h"
#include "types.h"

namespace GfxPaint {

QString buildFilterString(QList<QByteArray> imageFormats, QString allLabel = QString("supported"));

QWidget *centringWidget(QWidget *const widget);

// Builtin version doesn't discard key repeats
class KeyEventTransition : public QEventTransition {
public:
    KeyEventTransition(QState *const sourceState = nullptr) : QEventTransition(sourceState) {}
    KeyEventTransition(QObject *object, QEvent::Type type, int key, QState *sourceState = nullptr) :
        QEventTransition(object, type, sourceState),
        m_key(key)
    {}
    int key() const { return m_key; }
    void setKey(int key) { m_key = key; }
    Qt::KeyboardModifiers modifierMask() const { return m_modifierMask; }
    void setModifierMask(Qt::KeyboardModifiers modifierMask) { m_modifierMask = modifierMask; }

protected:
    virtual bool eventTest(QEvent *event) override {
        const bool baseResult = QEventTransition::eventTest(event);
        QStateMachine::WrappedEvent *const wrappedEvent = static_cast<QStateMachine::WrappedEvent *>(event);
        QKeyEvent *const keyEvent = static_cast<QKeyEvent *>(wrappedEvent->event());
        return baseResult &&
                keyEvent->key() == m_key &&
                (m_modifierMask == Qt::NoModifier || keyEvent->modifiers() & m_modifierMask) &&
                !keyEvent->isAutoRepeat();
    }

    int m_key;
    Qt::KeyboardModifiers m_modifierMask;
};

QString fileToString(const QString &path);

void stringMultiReplace(QString &string, const QMap<QString, QString> &replacements);

QTransform viewportTransform(const QSize size);

mat3 qTransformToMat3(const QTransform &transform);
QTransform qTransformFromMat3(const mat3 &matrix);

vec4 qColorToVec4(const QColor &qColor);
QColor qColorFromVec4(const vec4 &colour);

Buffer bufferFromImageFile(const QString &filename, Buffer *const palette = nullptr, Colour *const transparent = nullptr);

template <typename T>
T clamp(const T min, const T max, const T value) {
    return std::max(min, std::min(max, value));
}

} // namespace GfxPaint

#endif // UTIL_H
