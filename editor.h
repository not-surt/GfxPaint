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
#include "editorinputstate.h"

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

    EditorInputState inputState;
};

} // namespace GfxPaint

#endif // EDITOR_H
