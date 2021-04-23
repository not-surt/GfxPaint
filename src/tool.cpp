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
    for (Node *node : editor.editingContext().selectedNodes()) {
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
    for (Node *node : editor.editingContext().selectedNodes()) {
        const Traversal::State &state = editor.editingContext().states().value(node);
        BufferNode *const bufferNode = dynamic_cast<BufferNode *>(node);
        const EditingContext::BufferNodeContext *const bufferNodeContext = editor.editingContext().bufferNodeContext(node);
        if (bufferNode) {
//            bufferNodeContext->strokeBuffer->clear();
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
    points[0] = point.pos;
}

void RectTool::update(const QVector2D &viewportPos, const Point &point, const int mode)
{
    points[1] = point.pos;
}

void RectTool::end(const QVector2D &viewportPos, const Point &point, const int mode)
{
    update(viewportPos, point);
    for (Node *node : editor.editingContext().selectedNodes()) {
        const Traversal::State &state = editor.editingContext().states().value(node);
        BufferNode *const bufferNode = dynamic_cast<BufferNode *>(node);
        const EditingContext::BufferNodeContext *const bufferNodeContext = editor.editingContext().bufferNodeContext(node);
        if (bufferNode) {
            ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
            bufferNode->buffer.bindFramebuffer();

            RectProgram *const program = new RectProgram(false, bufferNode->buffer.format(), state.palette != nullptr, state.palette != nullptr ? state.palette->format() : Buffer::Format(), 0, RenderManager::composeModeDefault);
            program->render(points, editor.editingContext().colour(), GfxPaint::viewportTransform(bufferNode->buffer.size()) * bufferNode->transform().inverted(), &bufferNode->buffer, state.palette);
            delete program;
        }
    }
}

void RectTool::onCanvasPreview(const QVector2D &viewportPos, const Point &point, const int mode)
{
    auto savePoints = points;

    end(viewportPos, point);

    points = savePoints;
}

void RectTool::onTopPreview(const QVector2D &viewportPos, const Point &point, const int mode)
{
    ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
    Model *const markerModel = qApp->renderManager.models["planeMarker"];
    VertexColourModelProgram *const markerProgram = new VertexColourModelProgram(RenderedWidget::format, false, Buffer::Format(), 0, RenderManager::composeModeDefault);
    QMatrix4x4 markerTransform = editor.getViewportTransform();
    const QVector2D viewportPoint = editor.worldToViewport(points[0]);
    markerTransform.translate(viewportPoint.toVector3D());
    float markerSize = 16.0f;
    markerTransform.scale(markerSize, markerSize);
    markerProgram->render(markerModel, markerTransform, editor.getWidgetBuffer(), nullptr);
    delete markerProgram;
}

void EllipseTool::begin(const QVector2D &viewportPos, const Point &point, const int mode)
{
    points[0] = point.pos;
}

void EllipseTool::update(const QVector2D &viewportPos, const Point &point, const int mode)
{
    points[1] = point.pos;
}

void EllipseTool::end(const QVector2D &viewportPos, const Point &point, const int mode)
{
    update(viewportPos, point);
    for (Node *node : editor.editingContext().selectedNodes()) {
        const Traversal::State &state = editor.editingContext().states().value(node);
        BufferNode *const bufferNode = dynamic_cast<BufferNode *>(node);
        const EditingContext::BufferNodeContext *const bufferNodeContext = editor.editingContext().bufferNodeContext(node);
        if (bufferNode) {
            ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
            bufferNode->buffer.bindFramebuffer();

            EllipseProgram *const program = new EllipseProgram(true, bufferNode->buffer.format(), state.palette != nullptr, state.palette != nullptr ? state.palette->format() : Buffer::Format(), 0, RenderManager::composeModeDefault);
            program->render(points, editor.editingContext().colour(), GfxPaint::viewportTransform(bufferNode->buffer.size()) * bufferNode->transform().inverted(), &bufferNode->buffer, state.palette);
            delete program;
        }
    }
}

void EllipseTool::onCanvasPreview(const QVector2D &viewportPos, const Point &point, const int mode)
{
    auto savePoints = points;

    end(viewportPos, point);

    points = savePoints;
}

void EllipseTool::onTopPreview(const QVector2D &viewportPos, const Point &point, const int mode)
{
    ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
    Model *const markerModel = qApp->renderManager.models["planeMarker"];
    VertexColourModelProgram *const markerProgram = new VertexColourModelProgram(RenderedWidget::format, false, Buffer::Format(), 0, RenderManager::composeModeDefault);
    QMatrix4x4 markerTransform = editor.getViewportTransform();
    const QVector2D viewportPoint = editor.worldToViewport(points[0]);
    markerTransform.translate(viewportPoint.toVector3D());
    float markerSize = 16.0f;
    markerTransform.scale(markerSize, markerSize);
    markerProgram->render(markerModel, markerTransform, editor.getWidgetBuffer(), nullptr);
    delete markerProgram;
}

void ContourTool::begin(const QVector2D &viewportPos, const Point &point, const int mode)
{
    points.clear();
    points.push_back(point.pos);
}

void ContourTool::update(const QVector2D &viewportPos, const Point &point, const int mode)
{
    points.push_back(point.pos);
}

void ContourTool::end(const QVector2D &viewportPos, const Point &point, const int mode)
{
    update(viewportPos, point);
    for (Node *node : editor.editingContext().selectedNodes()) {
        const Traversal::State &state = editor.editingContext().states().value(node);
        BufferNode *const bufferNode = dynamic_cast<BufferNode *>(node);
        const EditingContext::BufferNodeContext *const bufferNodeContext = editor.editingContext().bufferNodeContext(node);
        if (bufferNode) {
            ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
            bufferNode->buffer.bindFramebuffer();

            ContourStencilProgram *const stencilProgram = new ContourStencilProgram(bufferNode->buffer.format(), state.palette != nullptr, state.palette != nullptr ? state.palette->format() : Buffer::Format(), 0, RenderManager::composeModeDefault);
            stencilProgram->render(points, editor.editingContext().colour(), GfxPaint::viewportTransform(bufferNode->buffer.size()) * bufferNode->transform().inverted(), &bufferNode->buffer, state.palette);

            QVector2D boundsMin = QVector2D(std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity());
            QVector2D boundsMax = QVector2D(-std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity());
            auto iterator = points.begin();
            while (iterator != points.end()) {
                boundsMin.setX(std::min(boundsMin.x(), iterator->x()));
                boundsMin.setY(std::min(boundsMin.y(), iterator->y()));
                boundsMax.setX(std::max(boundsMax.x(), iterator->x()));
                boundsMax.setY(std::max(boundsMax.y(), iterator->y()));
                ++iterator;
            }
            Model *const model = new Model(GL_TRIANGLE_STRIP, {2}, {
                                                                       boundsMin.x(), boundsMin.y(),
                                                                       boundsMax.x(), boundsMin.y(),
                                                                       boundsMin.x(), boundsMax.y(),
                                                                       boundsMax.x(), boundsMax.y()},
                                           {{0, 1, 2, 3}}, {4});
            SingleColourModelProgram *const modelProgram = new SingleColourModelProgram(bufferNode->buffer.format(), state.palette != nullptr, state.palette != nullptr ? state.palette->format() : Buffer::Format(), 0, RenderManager::composeModeDefault);
            modelProgram->render(model, editor.editingContext().colour(), GfxPaint::viewportTransform(bufferNode->buffer.size()) * bufferNode->transform().inverted(), &bufferNode->buffer, state.palette);
            delete modelProgram;

            stencilProgram->postRender();
            delete stencilProgram;
        }
    }
}

void ContourTool::onCanvasPreview(const QVector2D &viewportPos, const Point &point, const int mode)
{
    auto savePoints = points;

    end(viewportPos, point);

    points = savePoints;
}

void ContourTool::onTopPreview(const QVector2D &viewportPos, const Point &point, const int mode)
{
    ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
    Model *const markerModel = qApp->renderManager.models["planeMarker"];
    VertexColourModelProgram *const markerProgram = new VertexColourModelProgram(RenderedWidget::format, false, Buffer::Format(), 0, RenderManager::composeModeDefault);
    QMatrix4x4 markerTransform = editor.getViewportTransform();
    const QVector2D viewportPoint = editor.worldToViewport(points.front());
    markerTransform.translate(viewportPoint.toVector3D());
    float markerSize = 16.0f;
    markerTransform.scale(markerSize, markerSize);
    markerProgram->render(markerModel, markerTransform, editor.getWidgetBuffer(), nullptr);
    delete markerProgram;
}

void PickTool::begin(const QVector2D &viewportPos, const Point &point, const int mode)
{
    update(viewportPos, point);
}

void PickTool::update(const QVector2D &viewportPos, const Point &point, const int mode)
{
    for (Node *node : editor.editingContext().selectedNodes()) {
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

void TransformTargetOverrideTool::begin(const QVector2D &viewportPos, const Point &point, const int mode)
{
    oldTransformMode = static_cast<int>(editor.transformTarget());
    editor.setTransformTarget(static_cast<TransformTarget>(mode));
}

void TransformTargetOverrideTool::end(const QVector2D &viewportPos, const Point &point, const int mode)
{
    editor.setTransformTarget(static_cast<TransformTarget>(oldTransformMode));
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
    if (editor.transformTarget() == TransformTarget::View) {
        editor.setTransform(transform * editor.transform());
    }
    else {
        for (Node *node : editor.editingContext().selectedNodes()) {
            const Traversal::State &state = editor.editingContext().states().value(node);
            if (editor.transformTarget() == TransformTarget::Object) {
                SpatialNode *const spatialNode = dynamic_cast<SpatialNode *>(node);
                if (spatialNode) {
                    spatialNode->setTransform(state.parentTransform.inverted() * (editor.transform().inverted() * transform * editor.transform()) * state.parentTransform * spatialNode->transform());
                }
            }
            else if (editor.transformTarget() == TransformTarget::Brush) {

            }
        }
    }
    oldViewportPos = viewportPos;
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
    if (editor.transformTarget() == TransformTarget::View) {
        editor.setTransform(transform * editor.transform());
    }
    else {
        for (Node *node : editor.editingContext().selectedNodes()) {
            const Traversal::State &state = editor.editingContext().states().value(node);
            if (editor.transformTarget() == TransformTarget::Object) {
                SpatialNode *const spatialNode = dynamic_cast<SpatialNode *>(node);
                if (spatialNode) {
                    spatialNode->setTransform(state.parentTransform.inverted() * (editor.transform().inverted() * transform * editor.transform()) * state.parentTransform * spatialNode->transform());
                }
            }
            else if (editor.transformTarget() == TransformTarget::Brush) {

            }
        }
    }
    oldViewportPos = viewportPos;
}

void RotoZoomTool::onTopPreview(const QVector2D &viewportPos, const Point &point, const int mode)
{
    ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
    Model *const markerModel = qApp->renderManager.models["planeMarker"];
    VertexColourModelProgram *const markerProgram = new VertexColourModelProgram(RenderedWidget::format, false, Buffer::Format(), 0, RenderManager::composeModeDefault);
    QMatrix4x4 markerTransform = editor.getViewportTransform();
    float markerSize = 16.0f;
    markerTransform.scale(markerSize, markerSize);
    markerProgram->render(markerModel, markerTransform, editor.getWidgetBuffer(), nullptr);
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
