#include "tool.h"

#include "editor.h"

namespace GfxPaint {

void StrokeTool::begin(const QVector2D &viewportPos, const Point &point, const int mode) {
    strokePoints = {};
    strokeOffset = 0.0;
    strokePoints.add(point);
}

void StrokeTool::update(const QVector2D &viewportPos, const Point &point, const int mode) {
    const Stroke::Point &prevWorldPoint = strokePoints.points.last();
    const Stroke::Point worldPoint = strokePoints.add(point);
    for (auto index : editor.editingContext().selectionModel().selectedRows()) {
        Node *node = static_cast<Node *>(index.internalPointer());
        const Traversal::State &state = editor.editingContext().states().value(node);
        BufferNode *const bufferNode = dynamic_cast<BufferNode *>(node);
        const EditingContext::BufferNodeContext *const bufferNodeContext = editor.editingContext().bufferNodeContext(node);
        if (bufferNode) {
            ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
            bufferNode->buffer.bindFramebuffer();
            const Brush &brush = editor.editingContext().brush();
            Stroke stroke;
            strokeOffset = editor.strokeSegmentDabs(prevWorldPoint, worldPoint, brush.dab.size, brush.stroke.absoluteSpacing, brush.stroke.proportionalSpacing, strokeOffset, stroke);
            // TODO: instancing?
            for (auto point : stroke.points) {
                const QVector2D snappedPoint = editor.pixelSnap(point.pos);
                Brush::Dab dab(brush.dab);
                dab.size *= point.pressure;
                dab.angle += point.quaternion.toEulerAngles().z();
                editor.drawDab(dab, editor.editingContext().colour(), *bufferNode, snappedPoint);
            }

            // Draw to stroke buffer
//            bufferNodeContext->strokeBuffer->bindFramebuffer();
//            for (auto pos : points) {
//                const QPointF snappedPoint = editor.pixelSnap(pos);
//                editor.drawDab(brush.dab, editor.editingContext().colour(), *bufferNode, snappedPoint);
//            }
        }
    }
}

void StrokeTool::end(const QVector2D &viewportPos, const Point &point, const int mode) {
    if (strokePoints.points.count() == 1) update(viewportPos, point);

    // Clear stroke buffer
    for (auto index : editor.editingContext().selectionModel().selectedRows()) {
        Node *node = static_cast<Node *>(index.internalPointer());
        const Traversal::State &state = editor.editingContext().states().value(node);
        BufferNode *const bufferNode = dynamic_cast<BufferNode *>(node);
        const EditingContext::BufferNodeContext *const bufferNodeContext = editor.editingContext().bufferNodeContext(node);
        if (bufferNode) {
            bufferNodeContext->strokeBuffer->clear();
        }
    }
}

void StrokeTool::onCanvasPreview(const QVector2D &viewportPos, const Point &point, const int mode)
{
    auto savePoints = strokePoints;
    auto saveOffset = strokeOffset;

    begin(viewportPos, point);
    end(viewportPos, point);

    strokePoints = savePoints;
    strokeOffset = saveOffset;
}

void RectTool::begin(const QVector2D &viewportPos, const Point &point, const int mode)
{
    qDebug() << "RectTool::begin";
    points[0] = point.pos;
}

void RectTool::update(const QVector2D &viewportPos, const Point &point, const int mode)
{
    qDebug() << "RectTool::update";
    points[1] = point.pos;
    for (auto index : editor.editingContext().selectionModel().selectedRows()) {
        Node *node = static_cast<Node *>(index.internalPointer());
        const Traversal::State &state = editor.editingContext().states().value(node);
        BufferNode *const bufferNode = dynamic_cast<BufferNode *>(node);
        const EditingContext::BufferNodeContext *const bufferNodeContext = editor.editingContext().bufferNodeContext(node);
        if (bufferNode) {
            ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
            bufferNode->buffer.bindFramebuffer();

            // draw here!
        }
    }
}

void RectTool::end(const QVector2D &viewportPos, const Point &point, const int mode)
{
    qDebug() << "RectTool::end";
    points[1] = point.pos;
}

void RectTool::onCanvasPreview(const QVector2D &viewportPos, const Point &point, const int mode)
{
    auto savePoints = points;

    begin(viewportPos, point);
    end(viewportPos, point);

    points = savePoints;
}

void PickTool::begin(const QVector2D &viewportPos, const Point &point, const int mode)
{
    update(viewportPos, point);
}

void PickTool::update(const QVector2D &viewportPos, const Point &point, const int mode)
{
    for (auto index : editor.editingContext().selectionModel().selectedRows()) {
        Node *node = static_cast<Node *>(index.internalPointer());
        const Traversal::State &state = editor.editingContext().states().value(node);
        EditingContext::BufferNodeContext *const bufferNodeContext = editor.editingContext().bufferNodeContext(node);
        BufferNode *const bufferNode = dynamic_cast<BufferNode *>(node);
        if (bufferNode) {
            const QVector2D bufferPoint = QVector2D(state.transform.inverted().map(point.pos.toPointF()));
            ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
            editor.setColour(bufferNodeContext->colourPickProgram->pick(&bufferNode->buffer, bufferNode->indexed ? state.palette : nullptr, bufferPoint));
        }
    }
}

void PickTool::end(const QVector2D &viewportPos, const Point &point, const int mode)
{
}

void PanTool::begin(const QVector2D &viewportPos, const Point &point, const int mode)
{
    oldViewportPos = viewportPos;
}

void PanTool::update(const QVector2D &viewportPos, const Point &point, const int mode)
{
    const QVector2D translation = viewportPos - oldViewportPos;
    QMatrix4x4 transform;
    transform.translate(QVector3D(translation));
    if (editor.transformMode() == TransformMode::View) {
        editor.setTransform(transform * editor.transform());
    }
    else {
        for (auto index : editor.editingContext().selectionModel().selectedRows()) {
            Node *node = static_cast<Node *>(index.internalPointer());
            const Traversal::State &state = editor.editingContext().states().value(node);
            if (editor.transformMode() == TransformMode::Object) {
                SpatialNode *const spatialNode = dynamic_cast<SpatialNode *>(node);
                if (spatialNode) {
                    spatialNode->setTransform(state.parentTransform.inverted() * (editor.transform().inverted() * transform * editor.transform()) * state.parentTransform * spatialNode->transform());
                }
            }
            else if (editor.transformMode() == TransformMode::Brush) {

            }
        }
    }
    oldViewportPos = viewportPos;
}

void PanTool::end(const QVector2D &viewportPos, const Point &point, const int mode)
{
}

void RotoZoomTool::begin(const QVector2D &viewportPos, const Point &point, const int mode)
{
    oldViewportPos = viewportPos;
}

void RotoZoomTool::update(const QVector2D &viewportPos, const Point &point, const int mode)
{
    const Mode toolMode = static_cast<Mode>(mode);
    const bool rotate = (toolMode == Mode::RotoZoom || toolMode == Mode::Rotate);
    const bool zoom = (toolMode == Mode::RotoZoom || toolMode == Mode::Zoom);
    const QMatrix4x4 transform = transformPointToPoint(QVector2D(0.0f, 0.0f), oldViewportPos, viewportPos, rotate, zoom);
    if (editor.transformMode() == TransformMode::View) {
        editor.setTransform(transform * editor.transform());
    }
    else {
        for (auto index : editor.editingContext().selectionModel().selectedRows()) {
            Node *node = static_cast<Node *>(index.internalPointer());
            const Traversal::State &state = editor.editingContext().states().value(node);
            if (editor.transformMode() == TransformMode::Object) {
                SpatialNode *const spatialNode = dynamic_cast<SpatialNode *>(node);
                if (spatialNode) {
                    spatialNode->setTransform(state.parentTransform.inverted() * (editor.transform().inverted() * transform * editor.transform()) * state.parentTransform * spatialNode->transform());
                }
            }
            else if (editor.transformMode() == TransformMode::Brush) {

            }
        }
    }
    oldViewportPos = viewportPos;
}

void RotoZoomTool::end(const QVector2D &viewportPos, const Point &point, const int mode)
{

}

void RotoZoomTool::onTopPreview(const QVector2D &viewportPos, const Point &point, const int mode)
{
    ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
    Model *const markerModel = qApp->renderManager.models["planeMarker"];
    ModelProgram *const markerProgram = new ModelProgram(RenderedWidget::format, false, Buffer::Format(), 0, RenderManager::composeModeDefault);
    QMatrix4x4 markerTransform;
    float markerSize = 32.0f;
    markerTransform.scale(markerSize / (float)editor.width(), markerSize / (float)editor.height());
    markerProgram->render(markerModel, {}, markerTransform, editor.getWidgetBuffer(), nullptr);
    delete markerProgram;
}

void ZoomTool::wheel(const QVector2D &viewportPos, const QVector2D &delta, const int mode)
{
    const float scaling = std::pow(2.0f, delta.y());
    QMatrix4x4 transform = editor.transform();
    rotateScaleAtOrigin(transform, 0.0, scaling, viewportPos);
    editor.setTransform(transform);
}

void RotateTool::wheel(const QVector2D &viewportPos, const QVector2D &delta, const int mode)
{
    const float rotation = -15.0f * delta.y();
    QMatrix4x4 transform = editor.transform();
    rotateScaleAtOrigin(transform, rotation, 1.0f, viewportPos);
    editor.setTransform(transform);
}

} // namespace GfxPaint
