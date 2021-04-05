#include "tool.h"

#include "editor.h"

namespace GfxPaint {

void StrokeTool::begin(const QVector2D viewportPos, const Point &point) {
    strokePoints = {};
    strokeOffset = 0.0;
    strokePoints.add(point);
}

void StrokeTool::update(const QVector2D viewportPos, const Point &point) {
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
            Stroke points;
            strokeOffset = editor.strokeSegmentDabs(prevWorldPoint, worldPoint, brush.dab.size, brush.stroke.absoluteSpacing, brush.stroke.proportionalSpacing, strokeOffset, points);
            for (auto point : points.points) {
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

void StrokeTool::end(const QVector2D viewportPos, const Point &point) {
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

void StrokeTool::preview(const QVector2D viewportPos, const Point &point)
{
    auto savePoints = strokePoints;
    auto saveOffset = strokeOffset;

    begin(viewportPos, point);
    end(viewportPos, point);

    strokePoints = savePoints;
    strokeOffset = saveOffset;
}

void PickTool::begin(const QVector2D viewportPos, const Point &point)
{
    update(viewportPos, point);
}

void PickTool::update(const QVector2D viewportPos, const Point &point)
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

void PickTool::end(const QVector2D viewportPos, const Point &point)
{
}

void PanTool::begin(const QVector2D viewportPos, const Point &point)
{
    oldViewportPos = viewportPos;
}

void PanTool::update(const QVector2D viewportPos, const Point &point)
{
    if (editor.editingContext().selectionModel().selectedRows().isEmpty()) {
        const QVector2D translation = editor.viewportToWorld(viewportPos) - editor.viewportToWorld(oldViewportPos);
        QMatrix4x4 workMatrix;
        workMatrix.translate(QVector3D(translation));
        if (editor.transformMode() == TransformMode::View) {
            editor.setTransform(editor.transform() * workMatrix);
        }
    }
    else {
        for (auto index : editor.editingContext().selectionModel().selectedRows()) {
            Node *node = static_cast<Node *>(index.internalPointer());
            const Traversal::State &state = editor.editingContext().states().value(node);
            const QVector2D translation = editor.viewportToWorld(viewportPos) - editor.viewportToWorld(oldViewportPos);
            QMatrix4x4 workMatrix;
            workMatrix.translate(QVector3D(translation));
            if (editor.transformMode() == TransformMode::View) {
                editor.setTransform(editor.transform() * workMatrix);
            }
            else if (editor.transformMode() == TransformMode::Object) {
                SpatialNode *const spatialNode = dynamic_cast<SpatialNode *>(node);
                if (spatialNode) {
                    const QVector2D translation = QVector2D(state.parentTransform.inverted().map(editor.viewportToWorld(viewportPos).toPointF())) - QVector2D(state.parentTransform.inverted().map(editor.viewportToWorld(oldViewportPos).toPointF()));
                    QMatrix4x4 workMatrix;
                    workMatrix.translate(QVector3D(translation));
                    spatialNode->setTransform(spatialNode->transform() * workMatrix);
                }
            }
            else if (editor.transformMode() == TransformMode::Brush) {

            }
        }
    }
    oldViewportPos = viewportPos;
}

void PanTool::end(const QVector2D viewportPos, const Point &point)
{
}

void RotoZoomTool::begin(const QVector2D viewportPos, const Point &point)
{
    oldViewportPos = viewportPos;
}

void RotoZoomTool::update(const QVector2D viewportPos, const Point &point)
{
    if (editor.editingContext().selectionModel().selectedRows().isEmpty()) {
        if (editor.transformMode() == TransformMode::View) {
            editor.setTransform(transformPointToPoint(QVector2D(0., 0.), oldViewportPos, viewportPos) * editor.transform());
        }
    }
    for (auto index : editor.editingContext().selectionModel().selectedRows()) {
        Node *node = static_cast<Node *>(index.internalPointer());
        const Traversal::State &state = editor.editingContext().states().value(node);
        if (editor.transformMode() == TransformMode::View) {
            editor.setTransform(transformPointToPoint(QVector2D(0., 0.), oldViewportPos, viewportPos) * editor.transform());
        }
        else if (editor.transformMode() == TransformMode::Object) {
            SpatialNode *const spatialNode = dynamic_cast<SpatialNode *>(node);
            if (spatialNode) {
                const QMatrix4x4 transformSpace = (state.parentTransform * editor.transform()).inverted();
                spatialNode->setTransform(transformPointToPoint(QVector2D(transformSpace.map(QPointF(0., 0.))), QVector2D(transformSpace.map(oldViewportPos.toPointF())), QVector2D(transformSpace.map(viewportPos.toPointF()))) * spatialNode->transform());
            }
        }
        else if (editor.transformMode() == TransformMode::Brush) {

        }
    }
    oldViewportPos = viewportPos;
}

void RotoZoomTool::end(const QVector2D viewportPos, const Point &point)
{

}

void ZoomTool::wheel(const QVector2D viewportPos, const QVector2D delta)
{
    const float scaling = std::pow(2.0f, delta.y());
    QMatrix4x4 transform = editor.transform();
    rotateScaleAtOrigin(transform, 0.0, scaling, viewportPos);
    editor.setTransform(transform);
}

void RotateTool::wheel(const QVector2D viewportPos, const QVector2D delta)
{
    const float rotation = -15.0f * delta.y();
    QMatrix4x4 transform = editor.transform();
    rotateScaleAtOrigin(transform, rotation, 1.0f, viewportPos);
    editor.setTransform(transform);
}

} // namespace GfxPaint
