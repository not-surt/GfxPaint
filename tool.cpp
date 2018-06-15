#include "tool.h"

#include "editor.h"

namespace GfxPaint {

void StrokeTool::begin(const QPointF point) {
    strokePoints.clear();
    strokeOffset = 0.0;
    strokePoints.append(editor.viewportToWorld(point));
    qDebug() << "StrokeTool::begin";/////////////////////////////////////////////////
}

void StrokeTool::update(const QPointF point) {
    const QPointF &prevWorldPoint = strokePoints.last();
    const QPointF worldPoint = editor.viewportToWorld(point);
    strokePoints.append(worldPoint);
    for (auto index : editor.editingContext().selectionModel().selectedRows()) {
        Node *node = static_cast<Node *>(index.internalPointer());
        const Traversal::State &state = editor.editingContext().states().value(node);
        BufferNode *const bufferNode = dynamic_cast<BufferNode *>(node);
        if (bufferNode) {
            ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
            bufferNode->buffer.bindFramebuffer();
            const Brush &brush = editor.editingContext().brush();
            QList<QPointF> points;
            strokeOffset = editor.strokeSegmentDabs(prevWorldPoint, worldPoint, editor.editingContext().brush().stroke.absoluteSpacing.x(), strokeOffset, points);
            for (auto point : points) {
                const QPointF snappedPoint = editor.pixelSnap(point);
                editor.drawDab(brush.dab, editor.editingContext().colour(), *bufferNode, snappedPoint);
            }
        }
    }
}

void StrokeTool::end(const QPointF point) {
    if (strokePoints.count() == 1) update(point);
}

void PickTool::begin(const QPointF point)
{
    update(point);
}

void PickTool::update(const QPointF point)
{
    const QPointF worldPoint = editor.viewportToWorld(point);
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

void PickTool::end(const QPointF point)
{
}

void PanTool::begin(const QPointF point)
{
    oldPoint = point;
}

void PanTool::update(const QPointF point)
{
    const QPointF oldWorldPoint = editor.viewportToWorld(oldPoint);
    const QPointF worldPoint = editor.viewportToWorld(point);
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
                    const QPointF translation = state.parentTransform.inverted().map(worldPoint) - state.parentTransform.inverted().map(oldPoint);
                    spatialNode->setTransform(spatialNode->transform() * QTransform().translate(translation.x(), translation.y()));
                }
            }
            else if (editor.transformMode() == TransformMode::Brush) {

            }
        }
    }
    oldPoint = point;
}

void PanTool::end(const QPointF point)
{
}

void RotoZoomTool::begin(const QPointF point)
{
    oldPoint = point;
}

void RotoZoomTool::update(const QPointF point)
{
    if (editor.editingContext().selectionModel().selectedRows().isEmpty()) {
        if (editor.transformMode() == TransformMode::View) {
            editor.setTransform(editor.transform() * transformPointToPoint(QPointF(0., 0.), oldPoint, point));
        }
    }
    for (auto index : editor.editingContext().selectionModel().selectedRows()) {
        Node *node = static_cast<Node *>(index.internalPointer());
        const Traversal::State &state = editor.editingContext().states().value(node);
        if (editor.transformMode() == TransformMode::View) {
            editor.setTransform(editor.transform() * transformPointToPoint(QPointF(0., 0.), oldPoint, point));
        }
        else if (editor.transformMode() == TransformMode::Object) {
            SpatialNode *const spatialNode = dynamic_cast<SpatialNode *>(node);
            if (spatialNode) {
                const QTransform transformSpace = (state.parentTransform * editor.transform()).inverted();
                spatialNode->setTransform(spatialNode->transform() * transformPointToPoint(transformSpace.map(QPointF(0., 0.)), transformSpace.map(oldPoint), transformSpace.map(point)));
            }
        }
        else if (editor.transformMode() == TransformMode::Brush) {

        }
    }
    oldPoint = point;
}

void RotoZoomTool::end(const QPointF point)
{

}

void ZoomTool::wheel(const QPointF point, const QPointF delta)
{
    const qreal scaling = pow(2, delta.y());
    QTransform transform = editor.transform();
    rotateScaleAtOrigin(transform, 0.0, scaling, point);
    editor.setTransform(transform);
}

void RotateTool::wheel(const QPointF point, const QPointF delta)
{
    const qreal rotation = -15.0 * delta.y();
    QTransform transform = editor.transform();
    rotateScaleAtOrigin(transform, rotation, 1.0, point);
    editor.setTransform(transform);
}

} // namespace GfxPaint
