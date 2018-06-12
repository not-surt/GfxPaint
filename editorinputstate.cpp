#include "editorinputstate.h"

#include "editor.h"

namespace GfxPaint {

void EditorInputState::ToolState::onEntry(QEvent * const event) {
    if (m_tool && !activeTool) {
        QEvent *const unwrappedEvent = static_cast<QStateMachine::WrappedEvent *>(event)->event();
        if (unwrappedEvent->type() == QEvent::KeyPress || unwrappedEvent->type() == QEvent::MouseButtonPress) {
            QPointF pos;
            if (unwrappedEvent->type() == QEvent::KeyPress) {
                editor.grabMouse();
                pos = editor.mapFromGlobal(QCursor::pos());
            }
            else {
                const QMouseEvent *const mouseEvent = static_cast<const QMouseEvent *>(unwrappedEvent);
                pos = mouseEvent->localPos();
            }
            activeTool = m_tool;
            activeTool->begin(editor.mouseToViewport(pos));
        }
        else if (unwrappedEvent->type() == QEvent::Wheel) {
            QWheelEvent *const wheelEvent = static_cast<QWheelEvent *>(unwrappedEvent);
            const qreal stepSize = 15.0 * 8.0;
            m_tool->wheel(editor.mouseToViewport(wheelEvent->posF()), QPointF(wheelEvent->angleDelta().x() / stepSize, wheelEvent->angleDelta().y() / stepSize));
        }
        editor.update();
    }
    else if (activeTool) {
        QEvent *const unwrappedEvent = static_cast<QStateMachine::WrappedEvent *>(event)->event();
        if (unwrappedEvent->type() == QEvent::MouseMove) {
            const QMouseEvent *const mouseEvent = static_cast<const QMouseEvent *>(unwrappedEvent);
            activeTool->update(editor.mouseToViewport(mouseEvent->localPos()));
        }
        editor.update();
    }
}

void EditorInputState::ToolState::onExit(QEvent * const event) {
    if (activeTool) {
        QEvent *const unwrappedEvent = static_cast<QStateMachine::WrappedEvent *>(event)->event();
        if (unwrappedEvent->type() == QEvent::KeyRelease || unwrappedEvent->type() == QEvent::MouseButtonRelease) {
            QPointF pos;
            if (unwrappedEvent->type() == QEvent::KeyRelease) {
                pos = editor.mapFromGlobal(QCursor::pos());
            }
            else {
                const QMouseEvent *const mouseEvent = static_cast<const QMouseEvent *>(unwrappedEvent);
                pos = mouseEvent->localPos();
            }
            editor.releaseMouse();
            activeTool->end(editor.mouseToViewport(pos));
            activeTool = nullptr;
        }
        editor.update();
    }
}

EditorInputState::EditorInputState(Editor &editor, const Qt::MouseButton primaryToolMouseButton, const Qt::MouseButton secondaryToolMouseButton, const Qt::MouseButton transformMouseButton, const Qt::Key transformKey, const Qt::Key altToolKey, const Qt::Key altTransformKey) :
    QObject(),

    editor(editor),

    machine(QState::ParallelStates),

    mouseOverGroup(&machine), mouseOut(&mouseOverGroup), mouseIn(&mouseOverGroup),
    mouseEnter(&editor, QEvent::Enter), mouseLeave(&editor, QEvent::Leave),

    windowDeactivate(&editor, QEvent::WindowDeactivate),

    toolModeGroup(&machine), standardToolMode(&toolModeGroup), alternateToolMode(&toolModeGroup),
    alternateToolKeyPress(&editor, QEvent::KeyPress, altToolKey), alternateToolKeyRelease(&editor, QEvent::KeyRelease, altToolKey),

    transformGroup(&machine), standardTransform(&transformGroup), alternateTransform(&transformGroup),
    alternateTransformKeyPress(&editor, QEvent::KeyPress, altTransformKey), alternateTransformKeyRelease(&editor, QEvent::KeyRelease, altTransformKey),

    toolGroup(&machine), idle(&toolGroup), primaryTool(editor, &toolGroup), secondaryTool(editor, &toolGroup), transformTool(editor, &toolGroup), mouseWheelTool(editor, &toolGroup),
    primaryToolMousePress(&editor, QEvent::MouseButtonPress, primaryToolMouseButton), primaryToolMouseMove(&editor, QEvent::MouseMove, Qt::NoButton), primaryToolMouseRelease(&editor, QEvent::MouseButtonRelease, primaryToolMouseButton),
    secondaryToolMousePress(&editor, QEvent::MouseButtonPress, secondaryToolMouseButton), secondaryToolMouseMove(&editor, QEvent::MouseMove, Qt::NoButton), secondaryToolMouseRelease(&editor, QEvent::MouseButtonRelease, secondaryToolMouseButton),
    transformToolMousePress(&editor, QEvent::MouseButtonPress, transformMouseButton), transformToolMouseMove(&editor, QEvent::MouseMove, Qt::NoButton), transformToolMouseRelease(&editor, QEvent::MouseButtonRelease, transformMouseButton),
    transformToolKeyPress(&editor, QEvent::KeyPress, transformKey), transformKeyMove(&editor, QEvent::MouseMove, Qt::NoButton), transformToolKeyRelease(&editor, QEvent::KeyRelease, transformKey),
    mouseWheelIn(&editor, QEvent::Wheel), mouseWheelOut(&mouseWheelTool, &QState::entered),

    toolStates()
{
    mouseOverGroup.setInitialState(&mouseOut);
    mouseOut.addTransition(&mouseEnter);
    mouseEnter.setTargetState(&mouseIn);
    mouseIn.addTransition(&mouseLeave);
    mouseLeave.setTargetState(&mouseOut);

    toolModeGroup.setInitialState(&standardToolMode);
    standardToolMode.addTransition(&alternateToolKeyPress);
    alternateToolKeyPress.setTargetState(&alternateToolMode);
    alternateToolMode.addTransition(&alternateToolKeyRelease);
    alternateToolKeyRelease.setTargetState(&standardToolMode);

    transformGroup.setInitialState(&standardTransform);
    standardTransform.addTransition(&alternateTransformKeyPress);
    alternateTransformKeyPress.setTargetState(&alternateTransform);
    alternateTransform.addTransition(&alternateTransformKeyRelease);
    alternateTransformKeyRelease.setTargetState(&standardTransform);

    toolGroup.setInitialState(&idle);

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
    mouseWheelIn.setTargetState(&mouseWheelTool);
    idle.addTransition(&mouseWheelIn);
    mouseWheelOut.setTargetState(&idle);
    mouseWheelTool.addTransition(&mouseWheelOut);

    transformTool.addTransition(&windowDeactivate);
    windowDeactivate.setTargetState(&idle);

    editor.installEventFilter(this);

    primaryTool.setTool(&editor.strokeTool);
    transformTool.setTool(&editor.panTool);
    mouseWheelTool.setTool(&editor.zoomTool);

    QObject::connect(&alternateToolMode, &QState::entered, [this, &editor](){
        for (auto slot : editor.toolSlots) {

        }
        primaryTool.setTool(&editor.pickTool);
        transformTool.setTool(&editor.rotoZoomTool);
        mouseWheelTool.setTool(&editor.rotateTool);
    });
    QObject::connect(&alternateToolMode, &QState::exited, [this, &editor](){
        for (auto slot : editor.toolSlots) {

        }
        primaryTool.setTool(&editor.strokeTool);
        transformTool.setTool(&editor.panTool);
        mouseWheelTool.setTool(&editor.zoomTool);
    });

    machine.start();
}

} // namespace GfxPaint
