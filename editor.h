#ifndef EDITOR_H
#define EDITOR_H

#include "renderedwidget.h"

#include <QStateMachine>
#include <QMouseEventTransition>
#include <QKeyEventTransition>
#include <QElapsedTimer>
#include <QOpenGLShaderProgram>
#include <QItemSelectionModel>
#include "buffer.h"
#include "brush.h"
#include "rendermanager.h"
#include "scene.h"
#include "editingcontext.h"
#include "utils.h"

namespace GfxPaint {

enum class TransformMode {
    View,
    Object,
    Brush,
};

struct StrokeData {
    struct Point {
        QPointF pos;
        qreal presure;
    };

    QList<Point> points;
};

class Editor : public RenderedWidget
{
    Q_OBJECT

public:
    explicit Editor(Scene &scene, QWidget *parent = nullptr);
    Editor(const Editor &other);
    virtual ~Editor();

    void updateWindowTitle();
    void setDocumentFilename(const QString &filename);
    void setDocumentModified(const bool modified = true);
    QString label() const;

    virtual QSize sizeHint() const override { return QSize(64, 64); }

    EditingContext &editingContext() { return m_editingContext; }

    void activate();

    void insertNodes(const QList<Node *> &nodes);
    void removeSelectedNodes();
    void duplicateSelectedNodes();

    Scene &scene;
    SceneModel &model;

public slots:
    void setBrush(const Brush &brush);
    void setColour(const QColor &colour);
    void setTransformMode(const TransformMode transformMode);
    void setTransform(const QTransform &transform);
    void updateContext();

signals:
    void brushChanged(const Brush &brush);
    void colourChanged(const QColor &colour);
    void editingBufferChanged(Buffer *const buffer);
    void editingsNodeChanged(const QSet<Node *> &nodes);
    void transformModeChanged(const TransformMode transformMode);
    void transformChanged(const QTransform &transform);

protected:
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void wheelEvent(QWheelEvent *event) override;
    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual void keyReleaseEvent(QKeyEvent *event) override;

    void render() override;

    static void rotateScaleAtOrigin(QTransform &transform, const qreal rotation, const qreal scaling, const QPointF origin);
    static QTransform transformPointToPoint(const QPointF origin, const QPointF from, const QPointF to);
    bool handleMouseEvent(const Qt::KeyboardModifiers modifiers, const Qt::MouseButton button, const QPoint pos);
    qreal strokeSegmentDabs(const QPointF start, const QPointF end, const qreal spacing, const qreal offset, QList<QPointF> &output);
    void drawDab(const Dab &dab, const QColor &colour, BufferNode &node, const QPointF worldPos);

    EditingContext m_editingContext;

    QTransform cameraTransform;
    TransformMode transformMode;

    class InputState {
    public:
        InputState(QWidget *widget,
                   const Qt::MouseButton primaryToolMouseButton = Qt::LeftButton, const Qt::MouseButton secondaryToolMouseButton = Qt::RightButton, const Qt::MouseButton transformMouseButton = Qt::MiddleButton,
                   const Qt::Key transformKey = Qt::Key_Space, const Qt::Key altToolKey = Qt::Key_Control, const Qt::Key altTransformKey = Qt::Key_Alt) :
            machine(QState::ParallelStates),

            mouseOverGroup(&machine), mouseOut(&mouseOverGroup), mouseIn(&mouseOverGroup),
            mouseEnter(widget, QEvent::Enter), mouseLeave(widget, QEvent::Leave),

            toolGroup(&machine), standardTool(&toolGroup), alternateTool(&toolGroup),
            alternateToolKeyPress(widget, QEvent::KeyPress, altToolKey), alternateToolKeyRelease(widget, QEvent::KeyRelease, altToolKey),

            transformGroup(&machine), standardTransform(&transformGroup), alternateTransform(&transformGroup),
            alternateTransformKeyPress(widget, QEvent::KeyPress, altTransformKey), alternateTransformKeyRelease(widget, QEvent::KeyRelease, altTransformKey),

            modeGroup(&machine), idle(&modeGroup), primaryTool(&modeGroup), secondaryTool(&modeGroup), transform(&modeGroup),
            primaryToolMousePress(widget, QEvent::MouseButtonPress, primaryToolMouseButton), primaryToolMouseRelease(widget, QEvent::MouseButtonRelease, primaryToolMouseButton),
            secondaryToolMousePress(widget, QEvent::MouseButtonPress, secondaryToolMouseButton), secondaryToolMouseRelease(widget, QEvent::MouseButtonRelease, secondaryToolMouseButton),
            transformMousePress(widget, QEvent::MouseButtonPress, transformMouseButton), transformMouseRelease(widget, QEvent::MouseButtonRelease, transformMouseButton),
            transformKeyPress(widget, QEvent::KeyPress, transformKey), transformKeyRelease(widget, QEvent::KeyRelease, transformKey),

            windowDeactivate(widget, QEvent::WindowDeactivate)
        {
            mouseOverGroup.setInitialState(&mouseOut);
            mouseOut.addTransition(&mouseEnter);
            mouseEnter.setTargetState(&mouseIn);
            mouseIn.addTransition(&mouseLeave);
            mouseLeave.setTargetState(&mouseOut);

            toolGroup.setInitialState(&standardTool);
            standardTool.addTransition(&alternateToolKeyPress);
            alternateToolKeyPress.setTargetState(&alternateTool);
            alternateTool.addTransition(&alternateToolKeyRelease);
            alternateToolKeyRelease.setTargetState(&standardTool);

            transformGroup.setInitialState(&standardTransform);
            standardTransform.addTransition(&alternateTransformKeyPress);
            alternateTransformKeyPress.setTargetState(&alternateTransform);
            alternateTransform.addTransition(&alternateTransformKeyRelease);
            alternateTransformKeyRelease.setTargetState(&standardTransform);

            modeGroup.setInitialState(&idle);

            primaryToolMousePress.setTargetState(&primaryTool);
            idle.addTransition(&primaryToolMousePress);
            primaryToolMouseRelease.setTargetState(&idle);
            primaryTool.addTransition(&primaryToolMouseRelease);

            secondaryToolMousePress.setTargetState(&secondaryTool);
            idle.addTransition(&secondaryToolMousePress);
            secondaryToolMouseRelease.setTargetState(&idle);
            secondaryTool.addTransition(&secondaryToolMouseRelease);

            transformMousePress.setTargetState(&transform);
            idle.addTransition(&transformMousePress);
            transformMouseRelease.setTargetState(&idle);
            transform.addTransition(&transformMouseRelease);
            transformKeyPress.setTargetState(&transform);
            idle.addTransition(&transformKeyPress);
            transformKeyRelease.setTargetState(&idle);
            transform.addTransition(&transformKeyRelease);

            transform.addTransition(&windowDeactivate);
            windowDeactivate.setTargetState(&idle);

            QObject::connect(&transform, &QState::entered, widget, [widget](){widget->grabMouse();});
            QObject::connect(&transform, &QState::exited, widget, [widget](){widget->releaseMouse();});

            machine.start();
        }

        QStateMachine machine;

        QState mouseOverGroup;
        QState mouseOut;
        QState mouseIn;
        QEventTransition mouseEnter;
        QEventTransition mouseLeave;

        QState toolGroup;
        QState standardTool;
        QState alternateTool;
        KeyEventTransition alternateToolKeyPress;
        KeyEventTransition alternateToolKeyRelease;

        QState transformGroup;
        QState standardTransform;
        QState alternateTransform;
        KeyEventTransition alternateTransformKeyPress;
        KeyEventTransition alternateTransformKeyRelease;

        QState modeGroup;
        QState idle;
        QState primaryTool;
        QState secondaryTool;
        QState transform;
        QMouseEventTransition primaryToolMousePress;
        QMouseEventTransition primaryToolMouseRelease;
        QMouseEventTransition secondaryToolMousePress;
        QMouseEventTransition secondaryToolMouseRelease;
        QMouseEventTransition transformMousePress;
        QMouseEventTransition transformMouseRelease;
        KeyEventTransition transformKeyPress;
        KeyEventTransition transformKeyRelease;

        QEventTransition windowDeactivate; // Why SIGABRT when change order?

        QPointF oldPos;
    } inputState;
};

} // namespace GfxPaint

#endif // EDITOR_H
