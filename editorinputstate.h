#ifndef EDITORINPUTSTATE_H
#define EDITORINPUTSTATE_H

#include <QStateMachine>
#include <QEventTransition>
#include <QSignalTransition>
#include <QMouseEventTransition>
#include <QKeyEventTransition>

#include "utils.h"
#include "tool.h"

namespace GfxPaint {

class Editor;

class EditorInputState : private QObject
{
        Q_OBJECT

public:
    class ToolState : public QState
    {
    public:
        ToolState(Editor &editor, QState *const parent = nullptr) :
            QState(parent),
            editor(editor), m_tool(nullptr), activeTool(nullptr)
        {}

        Tool *tool() const  { return m_tool; }
        void setTool(Tool *tool) { m_tool = tool; }

    protected:
        virtual void onEntry(QEvent *const event) override;
        virtual void onExit(QEvent *const event) override;

        Editor &editor;
        Tool *m_tool;
        Tool *activeTool;
    };

    EditorInputState(Editor &editor,
                     const Qt::MouseButton primaryToolMouseButton = Qt::LeftButton, const Qt::MouseButton secondaryToolMouseButton = Qt::RightButton, const Qt::MouseButton transformMouseButton = Qt::MiddleButton,
                     const Qt::Key transformKey = Qt::Key_Space, const Qt::Key altToolKey = Qt::Key_Control, const Qt::Key altTransformKey = Qt::Key_Alt);

    Editor &editor;

    QStateMachine machine;

    QState mouseOverGroup;
    QState mouseOut;
    QState mouseIn;
    QEventTransition mouseEnter;
    QEventTransition mouseLeave;

    QEventTransition windowDeactivate; // TODO: why SIGABRT when change order?

    QState toolModeGroup;
    QState standardToolMode;
    QState alternateToolMode;
    KeyEventTransition alternateToolKeyPress;
    KeyEventTransition alternateToolKeyRelease;

    QState transformGroup;
    QState standardTransform;
    QState alternateTransform;
    KeyEventTransition alternateTransformKeyPress;
    KeyEventTransition alternateTransformKeyRelease;

    QState toolGroup;
    QState idle;
    ToolState primaryTool;
    ToolState secondaryTool;
    ToolState transformTool;
    ToolState mouseWheelTool;
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
    QEventTransition mouseWheelIn;
    QSignalTransition mouseWheelOut;

    QList<ToolState> toolStates;
};

} // namespace GfxPaint

#endif // EDITORINPUTSTATE_H
