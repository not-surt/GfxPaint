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

protected slots:
    void primaryToolMousePress() {
//        qDebug() << "Press" << inputState.mouseButtons << inputState.modifiers;
    }
    void primaryToolMouseMove() {
//        qDebug() << "Move" << inputState.mouseButtons << inputState.modifiers;
    }
    void primaryToolMouseRelease() {
//        qDebug() << "Release" << inputState.mouseButtons << inputState.modifiers;
    }

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
    bool handleMouseEvent(const QEvent::Type type, const Qt::KeyboardModifiers modifiers, const Qt::MouseButton button, const QPoint pos);
    qreal strokeSegmentDabs(const QPointF start, const QPointF end, const qreal spacing, const qreal offset, QList<QPointF> &output);
    void drawDab(const Dab &dab, const QColor &colour, BufferNode &node, const QPointF worldPos);

    EditingContext m_editingContext;

    QTransform cameraTransform;
    TransformMode transformMode;

    qreal strokeOffset;
    QList<QPointF> strokePoints;
    QPointF mousePointToWorld(const QPointF &point) {
        return cameraTransform.inverted().map(QPointF(mouseTransform.map(point)));
    }
    void strokeStart(const QPointF &point) {
        strokePoints.clear();
        strokeOffset = 0.0;
        strokePoints.append(mousePointToWorld(point));
    }
    void strokeAdd(const QPointF &point) {
        const QPointF &prevWorldPoint = strokePoints.last();
        const QPointF worldPoint = mousePointToWorld(point);
        strokePoints.append(worldPoint);
        for (auto index : m_editingContext.selectionModel().selectedRows()) {
            Node *node = static_cast<Node *>(index.internalPointer());
            const Traversal::State &state = m_editingContext.states().value(node);
            BufferNode *const bufferNode = dynamic_cast<BufferNode *>(node);
            if (bufferNode) {
                const Brush &brush = m_editingContext.brush();
                QList<QPointF> points;
                strokeOffset = strokeSegmentDabs(prevWorldPoint, worldPoint, brush.stroke.absoluteSpacing.x(), strokeOffset, points);
                for (auto point : points) {
                    drawDab(brush.dab, m_editingContext.colour(), *bufferNode, point);
                }
            }
        }
        update();
    }
    void strokeEnd(const QPointF &point) {
        if (strokePoints.isEmpty()) strokeAdd(point);
    }

    class InputState : private QObject {
    public:
        class ToolState : public QState {
        public:
            ToolState(Editor *const editor ,QState *const parent = nullptr) :
                QState(parent),
                editor(editor)
            {

            }

        protected:
            virtual void onEntry(QEvent *const event) override {
                QEvent *const unwrappedEvent = static_cast<QStateMachine::WrappedEvent *>(event)->event();
//                qDebug() << "onEntry" << event;
                if (unwrappedEvent->type() == QEvent::MouseButtonPress) {
                    QMouseEvent *const mouseEvent = static_cast<QMouseEvent *>(unwrappedEvent);
                    editor->strokeStart(mouseEvent->pos());
                    qDebug() << editor->strokePoints;
                }
                if (unwrappedEvent->type() == QEvent::MouseMove) {
                    QMouseEvent *const mouseEvent = static_cast<QMouseEvent *>(unwrappedEvent);
                    editor->strokeAdd(mouseEvent->pos());
                    qDebug() << editor->strokePoints;
                }
            }
            virtual void onExit(QEvent *const event) override {
                QEvent *const unwrappedEvent = static_cast<QStateMachine::WrappedEvent *>(event)->event();
//                qDebug() << "onExit" << event;
                if (unwrappedEvent->type() == QEvent::MouseButtonRelease) {
                    QMouseEvent *const mouseEvent = static_cast<QMouseEvent *>(unwrappedEvent);
                    editor->strokeEnd(mouseEvent->pos());
                    qDebug() << editor->strokePoints;
                }
            }

            Editor *const editor;
        };

        InputState(Editor *const editor,
                   const Qt::MouseButton primaryToolMouseButton = Qt::LeftButton, const Qt::MouseButton secondaryToolMouseButton = Qt::RightButton, const Qt::MouseButton transformMouseButton = Qt::MiddleButton,
                   const Qt::Key transformKey = Qt::Key_Space, const Qt::Key altToolKey = Qt::Key_Control, const Qt::Key altTransformKey = Qt::Key_Alt) :
            QObject(),

            editor(editor),

            machine(QState::ParallelStates),

            mouseOverGroup(&machine), mouseOut(&mouseOverGroup), mouseIn(&mouseOverGroup),
            mouseEnter(editor, QEvent::Enter), mouseLeave(editor, QEvent::Leave),

            toolGroup(&machine), standardTool(&toolGroup), alternateTool(&toolGroup),
            alternateToolKeyPress(editor, QEvent::KeyPress, altToolKey), alternateToolKeyRelease(editor, QEvent::KeyRelease, altToolKey),

            transformGroup(&machine), standardTransform(&transformGroup), alternateTransform(&transformGroup),
            alternateTransformKeyPress(editor, QEvent::KeyPress, altTransformKey), alternateTransformKeyRelease(editor, QEvent::KeyRelease, altTransformKey),

            modeGroup(&machine), idle(&modeGroup), primaryTool(editor, &modeGroup), secondaryTool(&modeGroup), transform(&modeGroup),
            primaryToolMousePress(editor, QEvent::MouseButtonPress, primaryToolMouseButton), primaryToolMouseMove(editor, QEvent::MouseMove, Qt::NoButton), primaryToolMouseRelease(editor, QEvent::MouseButtonRelease, primaryToolMouseButton),
            secondaryToolMousePress(editor, QEvent::MouseButtonPress, secondaryToolMouseButton), secondaryToolMouseMove(editor, QEvent::MouseMove, Qt::NoButton), secondaryToolMouseRelease(editor, QEvent::MouseButtonRelease, secondaryToolMouseButton),
            transformMousePress(editor, QEvent::MouseButtonPress, transformMouseButton), transformMouseMove(editor, QEvent::MouseMove, Qt::NoButton), transformMouseRelease(editor, QEvent::MouseButtonRelease, transformMouseButton),
            transformKeyPress(editor, QEvent::KeyPress, transformKey), transformKeyMove(editor, QEvent::MouseMove, Qt::NoButton), transformKeyRelease(editor, QEvent::KeyRelease, transformKey),

            windowDeactivate(editor, QEvent::WindowDeactivate),

            modifiers(Qt::NoModifier), mouseButtons{}, keys{}, mousePos{}
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
            primaryToolMouseMove.setTargetState(&primaryTool);
            primaryTool.addTransition(&primaryToolMouseMove);
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

            QObject::connect(&transform, &QState::entered, editor, [editor](){editor->grabMouse();});
            QObject::connect(&transform, &QState::exited, editor, [editor](){editor->releaseMouse();});

//            QObject::connect(&primaryTool, &QState::entered, widget, [this, widget](){
//                qDebug() << "Enter";
//            });
//            QObject::connect(&primaryTool, &QState::exited, widget, [this, widget](){
//                qDebug() << "Exit";
//            });
            QObject::connect(&primaryToolMousePress, &QMouseEventTransition::triggered, editor, &Editor::primaryToolMousePress);
            QObject::connect(&primaryToolMouseMove, &QMouseEventTransition::triggered, editor, &Editor::primaryToolMouseMove);
            QObject::connect(&primaryToolMouseRelease, &QMouseEventTransition::triggered, editor, &Editor::primaryToolMouseRelease);
//            QObject::connect(&primaryToolMouseMove, &QMouseEventTransition::triggered, widget, [this, widget](){
//                qDebug() << "Move" << primaryToolMousePress.eventSource();
//            });
//            QObject::connect(&primaryToolMouseRelease, &QMouseEventTransition::triggered, widget, [this, widget](){
//                qDebug() << "Release" << primaryToolMousePress.eventSource();
//            });

            editor->installEventFilter(this);

            primaryTool.installEventFilter(this);
            primaryToolMousePress.installEventFilter(this);

            machine.start();
        }

        Editor *const editor;

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
        ToolState primaryTool;
        QState secondaryTool;
        QState transform;
        QMouseEventTransition primaryToolMousePress;
        QMouseEventTransition primaryToolMouseMove;
        QMouseEventTransition primaryToolMouseRelease;
        QMouseEventTransition secondaryToolMousePress;
        QMouseEventTransition secondaryToolMouseMove;
        QMouseEventTransition secondaryToolMouseRelease;
        QMouseEventTransition transformMousePress;
        QMouseEventTransition transformMouseMove;
        QMouseEventTransition transformMouseRelease;
        KeyEventTransition transformKeyPress;
        QMouseEventTransition transformKeyMove;
        KeyEventTransition transformKeyRelease;

        QEventTransition windowDeactivate; // TODO: why SIGABRT when change order?

        Qt::KeyboardModifiers modifiers;
        QSet<Qt::MouseButton> mouseButtons;
        QSet<int> keys;
        QPoint mousePos;

        QPointF oldPos;
        qreal strokeOffset;
        QList<QPointF> strokePoints;        

    private:
        virtual bool eventFilter(QObject *watched, QEvent *event) override {
            if (watched == editor) {
                static const QSet<QEvent::Type> keyEvents = {QEvent::KeyPress, QEvent::KeyRelease};
                static const QSet<QEvent::Type> mouseEvents = {QEvent::MouseButtonPress, QEvent::MouseMove, QEvent::MouseButtonRelease};
                if (keyEvents.contains(event->type())) {
                    const QKeyEvent *const keyEvent = static_cast<QKeyEvent *>(event);
                    modifiers = keyEvent->modifiers();
                    if (event->type() == QEvent::KeyPress) keys.insert(keyEvent->key());
                    else if (event->type() == QEvent::KeyRelease) keys.remove(keyEvent->key());
                }
                else if (mouseEvents.contains(event->type())) {
                    const QMouseEvent *const mouseEvent = static_cast<QMouseEvent *>(event);
                    mousePos = mouseEvent->pos();
                    if (event->type() == QEvent::MouseButtonPress) mouseButtons.insert(mouseEvent->button());
                    else if (event->type() == QEvent::MouseMove) ;
                    else if (event->type() == QEvent::MouseButtonRelease) mouseButtons.remove(mouseEvent->button());
                }
            }
            else if (watched == &primaryTool) {
                qDebug() << event;
            }
            else if (watched == &primaryToolMousePress) {
                qDebug() << event;
            }
            return false;
        }
    } inputState;
};

} // namespace GfxPaint

#endif // EDITOR_H
