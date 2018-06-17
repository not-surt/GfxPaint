#include "tool.h"

#include "editor.h"

namespace GfxPaint {

void StrokeTool::begin(const QPointF pos, const qreal pressure, const qreal rotation) {
    strokePoints.clear();
    strokeOffset = 0.0;
    strokePoints.append({editor.viewportToWorld(pos), pressure, rotation});
}

void StrokeTool::update(const QPointF pos, const qreal pressure, const qreal rotation) {
    const Point &prevWorldPoint = strokePoints.last();
    const Point worldPoint = {editor.viewportToWorld(pos), pressure, rotation};
    strokePoints.append(worldPoint);
    for (auto index : editor.editingContext().selectionModel().selectedRows()) {
        Node *node = static_cast<Node *>(index.internalPointer());
        const Traversal::State &state = editor.editingContext().states().value(node);
        BufferNode *const bufferNode = dynamic_cast<BufferNode *>(node);
        const EditingContext::BufferNodeContext *const bufferNodeContext = editor.editingContext().bufferNodeContext(node);
        if (bufferNode) {
            ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
            bufferNode->buffer.bindFramebuffer();
            const Brush &brush = editor.editingContext().brush();
            QList<Point> points;
            const QSizeF spacing(qMax(brush.stroke.absoluteSpacing.width() + brush.stroke.proportionalSpacing.width() * brush.dab.size.width(), 1.0),
                                 qMax(brush.stroke.absoluteSpacing.height() + brush.stroke.proportionalSpacing.height() * brush.dab.size.height(), 1.0));
            strokeOffset = editor.strokeSegmentDabs(prevWorldPoint.pos, prevWorldPoint.pressure, prevWorldPoint.rotation, worldPoint.pos, worldPoint.pressure, worldPoint.rotation, spacing, strokeOffset, points);
            for (auto point : points) {
                const QPointF snappedPoint = editor.pixelSnap(point.pos);
                Dab dab(brush.dab);
                dab.size *= pressure;
                dab.angle += rotation;
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

void StrokeTool::end(const QPointF pos, const qreal pressure, const qreal rotation) {
    if (strokePoints.count() == 1) update(pos, pressure, rotation);

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

void PickTool::begin(const QPointF pos, const qreal pressure, const qreal rotation)
{
    update(pos, pressure, rotation);
}

void PickTool::update(const QPointF pos, const qreal pressure, const qreal rotation)
{
    const QPointF worldPoint = editor.viewportToWorld(pos);
    for (auto index : editor.editingContext().selectionModel().selectedRows()) {
        Node *node = static_cast<Node *>(index.internalPointer());
        const Traversal::State &state = editor.editingContext().states().value(node);
        EditingContext::BufferNodeContext *const bufferNodeContext = editor.editingContext().bufferNodeContext(node);
        BufferNode *const bufferNode = dynamic_cast<BufferNode *>(node);
        if (bufferNode) {
            const QPointF bufferPoint = state.transform.inverted().map(worldPoint);
            ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
            editor.setColour(bufferNodeContext->colourPickProgram->pick(&bufferNode->buffer, bufferNode->indexed ? state.palette : nullptr, bufferPoint));
        }
    }
}

void PickTool::end(const QPointF pos, const qreal pressure, const qreal rotation)
{
}

void PanTool::begin(const QPointF pos, const qreal pressure, const qreal rotation)
{
    oldPos = pos;
}

void PanTool::update(const QPointF pos, const qreal pressure, const qreal rotation)
{
    const QPointF oldWorldPoint = editor.viewportToWorld(oldPos);
    const QPointF worldPoint = editor.viewportToWorld(pos);
    if (editor.editingContext().selectionModel().selectedRows().isEmpty()) {
        const QPointF translation = worldPoint - oldWorldPoint;
        const QTransform translationTransform = QTransform().translate(translation.x(), translation.y());
        if (editor.transformMode() == TransformMode::View) {
            editor.setTransform(translationTransform * editor.transform());
        }
    }
    else {
        for (auto index : editor.editingContext().selectionModel().selectedRows()) {
            Node *node = static_cast<Node *>(index.internalPointer());
            const Traversal::State &state = editor.editingContext().states().value(node);
            const QPointF translation = worldPoint - oldWorldPoint;
            const QTransform translationTransform = QTransform().translate(translation.x(), translation.y());
            if (editor.transformMode() == TransformMode::View) {
                editor.setTransform(translationTransform * editor.transform());
            }
            else if (editor.transformMode() == TransformMode::Object) {
                SpatialNode *const spatialNode = dynamic_cast<SpatialNode *>(node);
                if (spatialNode) {
                    const QPointF translation = state.parentTransform.inverted().map(worldPoint) - state.parentTransform.inverted().map(oldPos);
                    spatialNode->setTransform(spatialNode->transform() * QTransform().translate(translation.x(), translation.y()));
                }
            }
            else if (editor.transformMode() == TransformMode::Brush) {

            }
        }
    }
    oldPos = pos;
}

void PanTool::end(const QPointF pos, const qreal pressure, const qreal rotation)
{
}

void RotoZoomTool::begin(const QPointF pos, const qreal pressure, const qreal rotation)
{
    oldPos = pos;
}

void RotoZoomTool::update(const QPointF pos, const qreal pressure, const qreal rotation)
{
    if (editor.editingContext().selectionModel().selectedRows().isEmpty()) {
        if (editor.transformMode() == TransformMode::View) {
            editor.setTransform(editor.transform() * transformPointToPoint(QPointF(0., 0.), oldPos, pos));
        }
    }
    for (auto index : editor.editingContext().selectionModel().selectedRows()) {
        Node *node = static_cast<Node *>(index.internalPointer());
        const Traversal::State &state = editor.editingContext().states().value(node);
        if (editor.transformMode() == TransformMode::View) {
            editor.setTransform(editor.transform() * transformPointToPoint(QPointF(0., 0.), oldPos, pos));
        }
        else if (editor.transformMode() == TransformMode::Object) {
            SpatialNode *const spatialNode = dynamic_cast<SpatialNode *>(node);
            if (spatialNode) {
                const QTransform transformSpace = (state.parentTransform * editor.transform()).inverted();
                spatialNode->setTransform(spatialNode->transform() * transformPointToPoint(transformSpace.map(QPointF(0., 0.)), transformSpace.map(oldPos), transformSpace.map(pos)));
            }
        }
        else if (editor.transformMode() == TransformMode::Brush) {

        }
    }
    oldPos = pos;
}

void RotoZoomTool::end(const QPointF pos, const qreal pressure, const qreal rotation)
{

}

void ZoomTool::wheel(const QPointF pos, const QPointF delta)
{
    const qreal scaling = pow(2, delta.y());
    QTransform transform = editor.transform();
    rotateScaleAtOrigin(transform, 0.0, scaling, pos);
    editor.setTransform(transform);
}

void RotateTool::wheel(const QPointF pos, const QPointF delta)
{
    const qreal rotation = -15.0 * delta.y();
    QTransform transform = editor.transform();
    rotateScaleAtOrigin(transform, rotation, 1.0, pos);
    editor.setTransform(transform);
}

} // namespace GfxPaint
