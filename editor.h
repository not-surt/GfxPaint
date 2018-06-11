#ifndef EDITOR_H
#define EDITOR_H

#include "renderedwidget.h"

#include <QStateMachine>
#include <QMouseEventTransition>
#include <QKeyEventTransition>
#include <QElapsedTimer>
#include <QOpenGLShaderProgram>
#include <QItemSelectionModel>
#include <cmath>
#include "buffer.h"
#include "brush.h"
#include "rendermanager.h"
#include "scene.h"
#include "editingcontext.h"
#include "utils.h"
#include "application.h"
#include "tool.h"

namespace GfxPaint {

enum class TransformMode {
    View,
    Object,
    Brush,
};

class Editor : public RenderedWidget
{
    Q_OBJECT

public:
    explicit Editor(Scene &scene, QWidget *parent = nullptr);
    Editor(const Editor &other);
    virtual ~Editor() override;

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

    QPointF mouseToViewport(const QPointF &point) {
        return mouseTransform.map(point);
    }
    QPointF viewportToWorld(const QPointF &point) {
        return cameraTransform.inverted().map(point);
    }
    QPointF mouseToWorld(const QPointF &point) {
        return viewportToWorld(mouseToViewport(point));
    }
    float pixelSnapOffset(const PixelSnap pixelSnap, const float target, const float size) {
        switch (pixelSnap) {
        case PixelSnap::Centre: return 0.5f;
        case PixelSnap::Edge: return 0.0f;
        case PixelSnap::Both: return fabs(target - floor(target) - 0.5f) < 0.25f ? 0.5f : 1.0f;
        case PixelSnap::Auto: return lround(size) % 2 == 0 ? 0.0f : 0.5f;
        default: return target;
        }
    }
    QPointF pixelSnap(const QPointF target) {
        const Dab &dab = m_editingContext.brush().dab;
        const float offsetX = pixelSnapOffset(dab.pixelSnapX, target.x(), dab.size.width());
        const float offsetY = pixelSnapOffset(dab.pixelSnapY, target.y(), dab.size.height());
        return snap2d({offsetX, offsetY}, {1.0, 1.0}, target);
    }
    qreal strokeSegmentDabs(const QPointF start, const QPointF end, const qreal spacing, const qreal offset, QList<QPointF> &output);
    void drawDab(const Dab &dab, const Colour &colour, BufferNode &node, const QPointF worldPos);
    void drawSegment(const Dab &dab, const Stroke &stroke, const Colour &colour, BufferNode &node, const QPointF start, const QPointF end, const qreal offset);

    Scene &scene;
    SceneModel &model;

    StrokeTool strokeTool;
    PickTool pickTool;
    PanTool transformTool;
    RotoScaleTool rotoScaleTool;

    TransformMode transformMode() const { return m_transformMode; }
    QTransform transform() const { return cameraTransform; }

public slots:
    void setBrush(const Brush &brush);
    void setColour(const Colour &colour);
    void setTransformMode(const TransformMode m_transformMode);
    void setTransform(const QTransform &transform);
    void updateContext();

signals:
    void brushChanged(const Brush &brush);
    void colourChanged(const Colour &colour);
    void paletteChanged(Buffer *const palette);
    void transformModeChanged(const TransformMode m_transformMode);
    void transformChanged(const QTransform &transform);

protected:
    virtual void mousePressEvent(QMouseEvent *event) override;
    virtual void mouseReleaseEvent(QMouseEvent *event) override;
    virtual void mouseMoveEvent(QMouseEvent *event) override;
    virtual void wheelEvent(QWheelEvent *event) override;
    virtual void keyPressEvent(QKeyEvent *event) override;
    virtual void keyReleaseEvent(QKeyEvent *event) override;

    void render() override;

    bool handleMouseEvent(const QEvent::Type type, const Qt::KeyboardModifiers modifiers, const Qt::MouseButton button, const QPoint pos);

    EditingContext m_editingContext;

    QTransform cameraTransform;
    TransformMode m_transformMode;

    class InputState : private QObject {
    public:
        class ToolState : public QState {
        public:
            ToolState(Editor *const editor, QState *const parent = nullptr) :
                QState(parent),
                editor(editor), m_tool(nullptr)
            {}

            Tool *tool() const  {
                return m_tool;
            }
            void setTool(Tool *tool) {
                m_tool = tool;
            }

        protected:
            virtual void onEntry(QEvent *const event) override {
                if (m_tool) {
                    QEvent *const unwrappedEvent = static_cast<QStateMachine::WrappedEvent *>(event)->event();
                    if (unwrappedEvent->type() == QEvent::KeyPress || unwrappedEvent->type() == QEvent::MouseButtonPress) {
                        QPointF pos;
                        if (unwrappedEvent->type() == QEvent::KeyPress) {
                            editor->grabMouse();
                            pos = editor->mapFromGlobal(QCursor::pos());
                        }
                        else {
                            QMouseEvent *const mouseEvent = static_cast<QMouseEvent *>(unwrappedEvent);
                            pos = mouseEvent->pos();
                        }
                        m_tool->begin(editor->mouseToViewport(pos));
                    }
                    else if (unwrappedEvent->type() == QEvent::MouseMove) {
                        QMouseEvent *const mouseEvent = static_cast<QMouseEvent *>(unwrappedEvent);
                        QPointF pos = mouseEvent->pos();
                        m_tool->update(editor->mouseToViewport(pos));
                    }
                    editor->update();
                }
            }
            virtual void onExit(QEvent *const event) override {
                if (m_tool) {
                    QEvent *const unwrappedEvent = static_cast<QStateMachine::WrappedEvent *>(event)->event();
                    if (unwrappedEvent->type() == QEvent::KeyRelease || unwrappedEvent->type() == QEvent::MouseButtonRelease) {
                        QPointF pos;
                        if (unwrappedEvent->type() == QEvent::KeyRelease) {
                            editor->releaseMouse();
                            pos = editor->mapFromGlobal(QCursor::pos());
                        }
                        else {
                            QMouseEvent *const mouseEvent = static_cast<QMouseEvent *>(unwrappedEvent);
                            pos = mouseEvent->pos();
                        }
                        m_tool->end(editor->mouseToViewport(pos));
                    }
                    editor->update();
                }
            }

            Editor *const editor;
            Tool *m_tool;
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

            modeGroup(&machine), idle(&modeGroup), primaryTool(editor, &modeGroup), secondaryTool(editor, &modeGroup), transformTool(editor, &modeGroup),
            primaryToolMousePress(editor, QEvent::MouseButtonPress, primaryToolMouseButton), primaryToolMouseMove(editor, QEvent::MouseMove, Qt::NoButton), primaryToolMouseRelease(editor, QEvent::MouseButtonRelease, primaryToolMouseButton),
            secondaryToolMousePress(editor, QEvent::MouseButtonPress, secondaryToolMouseButton), secondaryToolMouseMove(editor, QEvent::MouseMove, Qt::NoButton), secondaryToolMouseRelease(editor, QEvent::MouseButtonRelease, secondaryToolMouseButton),
            transformToolMousePress(editor, QEvent::MouseButtonPress, transformMouseButton), transformToolMouseMove(editor, QEvent::MouseMove, Qt::NoButton), transformToolMouseRelease(editor, QEvent::MouseButtonRelease, transformMouseButton),
            transformToolKeyPress(editor, QEvent::KeyPress, transformKey), transformKeyMove(editor, QEvent::MouseMove, Qt::NoButton), transformToolKeyRelease(editor, QEvent::KeyRelease, transformKey),

            windowDeactivate(editor, QEvent::WindowDeactivate),

            toolStates(),

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
            secondaryToolMouseMove.setTargetState(&secondaryTool);
            secondaryTool.addTransition(&secondaryToolMouseMove);
            secondaryToolMouseRelease.setTargetState(&idle);
            secondaryTool.addTransition(&secondaryToolMouseRelease);

            transformToolMousePress.setTargetState(&transformTool);
            idle.addTransition(&transformToolMousePress);
            transformToolMouseMove.setTargetState(&transformTool);
            transformTool.addTransition(&transformToolMouseMove);
            transformToolMouseRelease.setTargetState(&idle);
            transformTool.addTransition(&transformToolMouseRelease);
            transformToolKeyPress.setTargetState(&transformTool);
            idle.addTransition(&transformToolKeyPress);
            transformToolKeyRelease.setTargetState(&idle);
            transformTool.addTransition(&transformToolKeyRelease);

            transformTool.addTransition(&windowDeactivate);
            windowDeactivate.setTargetState(&idle);

            editor->installEventFilter(this);

            primaryTool.setTool(&editor->strokeTool);
            secondaryTool.setTool(&editor->pickTool);
            transformTool.setTool(&editor->transformTool);
//            transformTool.setTool(&editor->rotoScaleTool);

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
        ToolState secondaryTool;
        ToolState transformTool;
        QMouseEventTransition primaryToolMousePress;
        QMouseEventTransition primaryToolMouseMove;
        QMouseEventTransition primaryToolMouseRelease;
        QMouseEventTransition secondaryToolMousePress;
        QMouseEventTransition secondaryToolMouseMove;
        QMouseEventTransition secondaryToolMouseRelease;
        QMouseEventTransition transformToolMousePress;
        QMouseEventTransition transformToolMouseMove;
        QMouseEventTransition transformToolMouseRelease;
        KeyEventTransition transformToolKeyPress;
        QMouseEventTransition transformKeyMove;
        KeyEventTransition transformToolKeyRelease;

        QEventTransition windowDeactivate; // TODO: why SIGABRT when change order?

        QList<ToolState> toolStates;

        Qt::KeyboardModifiers modifiers;
        QSet<Qt::MouseButton> mouseButtons;
        QSet<int> keys;
        QPoint mousePos;

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
            return false;
        }
    } inputState;
};

} // namespace GfxPaint

#endif // EDITOR_H
