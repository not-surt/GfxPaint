#ifndef EDITOR_H
#define EDITOR_H

#include "renderedwidget.h"

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

    friend class EditorInputState;

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
    PanTool panTool;
    RotoZoomTool rotoZoomTool;
    ZoomTool zoomTool;
    RotateTool rotateTool;

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
    void render() override;

    EditingContext m_editingContext;

    QTransform cameraTransform;
    TransformMode m_transformMode;

    QList<Tool *> tools;
    QList<std::tuple<Tool *, Tool *>> toolSlots;

    ToolSet toolSet;
    Tool *activeTool;
    bool mouseOver;

protected:
    void toolAbort() {
        releaseMouse();
        activeTool = nullptr;
    }
    Tool *toolSetTool(const Qt::KeyboardModifiers modifiers, const ToolTrigger trigger) {
        if (toolSet.contains(modifiers) && toolSet[modifiers].contains(trigger)) return toolSet[modifiers][trigger];
        else return nullptr;
    }

    virtual void mousePressEvent(QMouseEvent *const event) override
    {
        Tool *const tool = toolSetTool(event->modifiers(), {ToolTrigger::Type::MouseButton, event->button()});
        if (tool) {
            qDebug() << "begin!";//////////////////////////////////////
            activeTool = tool;
            tool->begin(mouseToViewport(event->localPos()));
            event->accept();
        }
    }
    virtual void mouseReleaseEvent(QMouseEvent *const event) override
    {
        Tool *const tool = toolSetTool(event->modifiers(), {ToolTrigger::Type::MouseButton, event->button()});
        if (tool) {
            qDebug() << "end!";//////////////////////////////////////
            tool->end(mouseToViewport(event->localPos()));
            activeTool = nullptr;
            event->accept();
        }
    }
    virtual void mouseMoveEvent(QMouseEvent *const event) override
    {
//        Tool *const tool = toolSetTool(event->modifiers(), {ToolTrigger::Type::MouseButton, event->button()});
//        if (tool) {
//            qDebug() << "update!";//////////////////////////////////////
//            tool->update(mouseToViewport(event->localPos()));
//            event->accept();
//        }
        if (activeTool) {
            qDebug() << "update!";//////////////////////////////////////
            activeTool->update(mouseToViewport(event->localPos()));
            event->accept();
        }
    }
    virtual void wheelEvent(QWheelEvent *const event) override
    {
        Tool *const tool = toolSetTool(event->modifiers(), {ToolTrigger::Type::MouseWheel, 0});
        if (tool) {
            qDebug() << "wheel!";//////////////////////////////////////
            static const qreal stepSize = 15.0 * 8.0;
            tool->wheel(mouseToViewport(event->posF()), {event->angleDelta().x() / stepSize, event->angleDelta().y() / stepSize});
            event->accept();
        }
    }
    virtual void keyPressEvent(QKeyEvent *const event) override
    {
        static const QSet<Qt::Key> modifiers = {Qt::Key_Shift, Qt::Key_Control, Qt::Key_Meta, Qt::Key_Alt, Qt::Key_AltGr};
        if (modifiers.contains(static_cast<Qt::Key>(event->key()))) {

        }
        if (!event->isAutoRepeat()) {
            Tool *const tool = toolSetTool(event->modifiers(), {ToolTrigger::Type::Key, event->key()});
            if (tool) {
                qDebug() << "begin!";//////////////////////////////////////
                activeTool = tool;
                grabMouse();
                tool->begin(mouseToViewport(mapFromGlobal(QCursor::pos())));
                event->accept();
            }
        }
    }
    virtual void keyReleaseEvent(QKeyEvent *const event) override
    {
        static const QSet<Qt::Key> modifiers = {Qt::Key_Shift, Qt::Key_Control, Qt::Key_Meta, Qt::Key_Alt, Qt::Key_AltGr};
        if (modifiers.contains(static_cast<Qt::Key>(event->key()))) {

        }
        if (!event->isAutoRepeat()) {
            Tool *const tool = toolSetTool(event->modifiers(), {ToolTrigger::Type::Key, event->key()});
            if (tool) {
                qDebug() << "end!";//////////////////////////////////////
                tool->end(mouseToViewport(mapFromGlobal(QCursor::pos())));
                releaseMouse();
                activeTool = nullptr;
                event->accept();
            }
        }
    }

    // QObject interface
public:
    virtual bool event(QEvent *const event) override
    {
        /*if (event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease) {

        }
        else if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonRelease || event->type() == QEvent::MouseMove) {

        }
        else if (event->type() == QEvent::Wheel) {

        }
        else */if (event->type() == QEvent::Enter) {
            mouseOver = true;
        }
        else if (event->type() == QEvent::Leave) {
            mouseOver = false;
        }
        else if (event->type() == QEvent::WindowDeactivate) {
            toolAbort();
            mouseOver = false;
        }
        else return RenderedWidget::event(event);
    }
};

} // namespace GfxPaint

#endif // EDITOR_H
