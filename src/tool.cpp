#include "tool.h"

#include "editor.h"

namespace GfxPaint {

void StrokeTool::begin(const Vec2 &viewportPos, const Point &point, const int mode) {
    strokePoints = {};
    strokeOffset = 0.0;
    strokePoints.add(point);
}

void StrokeTool::update(const Vec2 &viewportPos, const Point &point, const int mode) {
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
                const Vec2 snappedPoint = editor.pixelSnap(point.pos);
                Brush::Dab dab(brush.dab);
                dab.size *= point.pressure;
                dab.angle += point.quaternion.toEulerAngles().z();
                editor.drawDab(dab, editor.editingContext().colour(), *bufferNode, snappedPoint);
            }

            // Draw to stroke buffer
//            bufferNodeContext->strokeBuffer->bindFramebuffer();
//            for (auto pos : points) {
//                const Vector2D snappedPoint = editor.pixelSnap(pos);
//                editor.drawDab(brush.dab, editor.editingContext().colour(), *bufferNode, snappedPoint);
//            }
        }
    }
}

void StrokeTool::end(const Vec2 &viewportPos, const Point &point, const int mode) {
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

void StrokeTool::onCanvasPreview(const Vec2 &viewportPos, const Point &point, const bool isActive, const int mode)
{
    auto savePoints = strokePoints;
    auto saveOffset = strokeOffset;

    begin(viewportPos, point);
    end(viewportPos, point);

    strokePoints = savePoints;
    strokeOffset = saveOffset;
}

void RectTool::begin(const Vec2 &viewportPos, const Point &point, const int mode)
{
    points[0] = point.pos;
}

void RectTool::update(const Vec2 &viewportPos, const Point &point, const int mode)
{
    points[1] = point.pos;
}

void RectTool::end(const Vec2 &viewportPos, const Point &point, const int mode)
{
    update(viewportPos, point);
    for (Node *node : editor.editingContext().selectedNodes()) {
        const Traversal::State &state = editor.editingContext().states().value(node);
        BufferNode *const bufferNode = dynamic_cast<BufferNode *>(node);
        const EditingContext::BufferNodeContext *const bufferNodeContext = editor.editingContext().bufferNodeContext(node);
        if (bufferNode) {
            ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
            bufferNode->buffer.bindFramebuffer();

//            Vec2 offset = bufferNode->transform() * Vec2(0.0f, 0.0f);
//            Mat4 offsetTransform;
//            offsetTransform.translate(-offset);
//            Mat4 offsetTransform = bufferNode->transform().inverted();

            const Mat4 toolSpaceTransform = editor.toolSpace(*bufferNode, editor.editingContext().brush().dab.space);

            const Traversal::State &state = editor.editingContext().states().value(node);
            RectProgram *const program = new RectProgram(false, bufferNode->buffer.format(), state.palette != nullptr, state.palette != nullptr ? state.palette->format() : Buffer::Format(), 0, RenderManager::composeModeDefault);
            program->render(points, editor.editingContext().colour(), bufferNode->viewportTransform() * toolSpaceTransform, &bufferNode->buffer, state.palette);
            delete program;
        }
    }
}

void RectTool::onCanvasPreview(const Vec2 &viewportPos, const Point &point, const bool isActive, const int mode)
{
    if (isActive) {
        auto savePoints = points;

        end(viewportPos, point);

        points = savePoints;
    }
}

void RectTool::onTopPreview(const Vec2 &viewportPos, const Point &point, const bool isActive, const int mode)
{
    if (isActive) {
        ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
        Model *const markerModel = qApp->renderManager.models["planeMarker"];
        VertexColourModelProgram *const markerProgram = new VertexColourModelProgram(RenderedWidget::format, false, Buffer::Format(), 0, RenderManager::composeModeDefault);
        Mat4 markerTransform = editor.getViewportTransform();
        const Vec2 viewportPoint = editor.transform() * points[0];
        markerTransform.translate(viewportPoint.toVector3D());
        float markerSize = 16.0f;
        markerTransform.scale(markerSize, markerSize);
        markerProgram->render(markerModel, markerTransform, editor.getWidgetBuffer(), nullptr);
        delete markerProgram;
    }
}

void EllipseTool::begin(const Vec2 &viewportPos, const Point &point, const int mode)
{
    points[0] = point.pos;
}

void EllipseTool::update(const Vec2 &viewportPos, const Point &point, const int mode)
{
    points[1] = point.pos;
}

void EllipseTool::end(const Vec2 &viewportPos, const Point &point, const int mode)
{
    update(viewportPos, point);
    for (Node *node : editor.editingContext().selectedNodes()) {
        const Traversal::State &state = editor.editingContext().states().value(node);
        BufferNode *const bufferNode = dynamic_cast<BufferNode *>(node);
        const EditingContext::BufferNodeContext *const bufferNodeContext = editor.editingContext().bufferNodeContext(node);
        if (bufferNode) {
            ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
            bufferNode->buffer.bindFramebuffer();

            const Traversal::State &state = editor.editingContext().states().value(node);
            EllipseProgram *const program = new EllipseProgram(true, bufferNode->buffer.format(), state.palette != nullptr, state.palette != nullptr ? state.palette->format() : Buffer::Format(), 0, RenderManager::composeModeDefault);
            program->render(points, editor.editingContext().colour(), bufferNode->viewportTransform() * state.transform.inverted(), &bufferNode->buffer, state.palette);
            delete program;
        }
    }
}

void EllipseTool::onCanvasPreview(const Vec2 &viewportPos, const Point &point, const bool isActive, const int mode)
{
    if (isActive) {
        auto savePoints = points;

        end(viewportPos, point);

        points = savePoints;
    }
}

void EllipseTool::onTopPreview(const Vec2 &viewportPos, const Point &point, const bool isActive, const int mode)
{
    if (isActive) {
        ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
        Model *const markerModel = qApp->renderManager.models["planeMarker"];
        VertexColourModelProgram *const markerProgram = new VertexColourModelProgram(RenderedWidget::format, false, Buffer::Format(), 0, RenderManager::composeModeDefault);
        Mat4 markerTransform = editor.getViewportTransform();
        const Vec2 viewportPoint = editor.transform() * points[0];
        markerTransform.translate(viewportPoint.toVector3D());
        float markerSize = 16.0f;
        markerTransform.scale(markerSize, markerSize);
        markerProgram->render(markerModel, markerTransform, editor.getWidgetBuffer(), nullptr);
        delete markerProgram;
    }
}

void ContourTool::begin(const Vec2 &viewportPos, const Point &point, const int mode)
{
    points.clear();
    points.push_back(point.pos);
}

void ContourTool::update(const Vec2 &viewportPos, const Point &point, const int mode)
{
    points.push_back(point.pos);
}

void ContourTool::end(const Vec2 &viewportPos, const Point &point, const int mode)
{
    update(viewportPos, point);
    for (Node *node : editor.editingContext().selectedNodes()) {
        const Traversal::State &state = editor.editingContext().states().value(node);
        BufferNode *const bufferNode = dynamic_cast<BufferNode *>(node);
        const EditingContext::BufferNodeContext *const bufferNodeContext = editor.editingContext().bufferNodeContext(node);
        if (bufferNode) {
            ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
            bufferNode->buffer.bindFramebuffer();

            const Traversal::State &state = editor.editingContext().states().value(node);
            ContourStencilProgram *const stencilProgram = new ContourStencilProgram(bufferNode->buffer.format(), state.palette != nullptr, state.palette != nullptr ? state.palette->format() : Buffer::Format(), 0, RenderManager::composeModeDefault);
            stencilProgram->render(points, editor.editingContext().colour(), bufferNode->viewportTransform() * state.transform.inverted(), &bufferNode->buffer, state.palette);

            Vec2 boundsMin = Vec2(std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity());
            Vec2 boundsMax = Vec2(-std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity());
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
            modelProgram->render(model, editor.editingContext().colour(), bufferNode->viewportTransform() * state.transform.inverted(), &bufferNode->buffer, state.palette);
            delete modelProgram;

            stencilProgram->postRender();
            delete stencilProgram;
        }
    }
}

void ContourTool::onCanvasPreview(const Vec2 &viewportPos, const Point &point, const bool isActive, const int mode)
{
    if (isActive) {
        auto savePoints = points;

        end(viewportPos, point);

        points = savePoints;
    }
}

void ContourTool::onTopPreview(const Vec2 &viewportPos, const Point &point, const bool isActive, const int mode)
{
    if (isActive) {
        ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
        Model *const markerModel = qApp->renderManager.models["planeMarker"];
        VertexColourModelProgram *const markerProgram = new VertexColourModelProgram(RenderedWidget::format, false, Buffer::Format(), 0, RenderManager::composeModeDefault);
        Mat4 markerTransform = editor.getViewportTransform();
        const Vec2 viewportPoint = editor.transform() * points.front();
        markerTransform.translate(viewportPoint.toVector3D());
        float markerSize = 16.0f;
        markerTransform.scale(markerSize, markerSize);
        markerProgram->render(markerModel, markerTransform, editor.getWidgetBuffer(), nullptr);
        delete markerProgram;
    }
}

void PickTool::begin(const Vec2 &viewportPos, const Point &point, const int mode)
{
    update(viewportPos, point);
}

void PickTool::update(const Vec2 &viewportPos, const Point &point, const int mode)
{
    for (Node *node : editor.editingContext().selectedNodes()) {
        const Traversal::State &state = editor.editingContext().states().value(node);
        EditingContext::BufferNodeContext *const bufferNodeContext = editor.editingContext().bufferNodeContext(node);
        BufferNode *const bufferNode = dynamic_cast<BufferNode *>(node);
        if (bufferNode) {
            const Vec2 bufferPoint = state.transform.inverted() * point.pos;
            ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
            editor.setColour(bufferNodeContext->colourPickProgram->pick(&bufferNode->buffer, bufferNode->indexed ? state.palette : nullptr, bufferPoint));
        }
    }
}

void TransformTargetOverrideTool::begin(const Vec2 &viewportPos, const Point &point, const int mode)
{
    oldTransformMode = static_cast<int>(editor.transformTarget());
    editor.setTransformTarget(static_cast<TransformTarget>(mode));
}

void TransformTargetOverrideTool::end(const Vec2 &viewportPos, const Point &point, const int mode)
{
    editor.setTransformTarget(static_cast<TransformTarget>(oldTransformMode));
}

void PanTool::begin(const Vec2 &viewportPos, const Point &point, const int mode)
{
    oldViewportPos = viewportPos;
}

void PanTool::update(const Vec2 &viewportPos, const Point &point, const int mode)
{
    const Vec2 translation = viewportPos - oldViewportPos;
    Mat4 transform;
    transform.translate(translation);
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

void RotoZoomTool::begin(const Vec2 &viewportPos, const Point &point, const int mode)
{
    oldViewportPos = viewportPos;
}

void RotoZoomTool::update(const Vec2 &viewportPos, const Point &point, const int mode)
{
    const Mode toolMode = static_cast<Mode>(mode);
    const bool rotate = (toolMode == Mode::RotoZoom || toolMode == Mode::Rotate);
    const bool zoom = (toolMode == Mode::RotoZoom || toolMode == Mode::Zoom);
    const Mat4 transform = transformPointToPoint(Vec2(0.0f, 0.0f), oldViewportPos, viewportPos, rotate, zoom);
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

void RotoZoomTool::onTopPreview(const Vec2 &viewportPos, const Point &point, const bool isActive, const int mode)
{
    if (isActive) {
        ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
        Model *const markerModel = qApp->renderManager.models["planeMarker"];
        VertexColourModelProgram *const markerProgram = new VertexColourModelProgram(RenderedWidget::format, false, Buffer::Format(), 0, RenderManager::composeModeDefault);
        Mat4 markerTransform = editor.getViewportTransform();
        float markerSize = 16.0f;
        markerTransform.scale(markerSize, markerSize);
        markerProgram->render(markerModel, markerTransform, editor.getWidgetBuffer(), nullptr);
        delete markerProgram;
    }
}

void ZoomTool::wheel(const Vec2 &viewportPos, const Vec2 &delta, const int mode)
{
    const float scaling = std::pow(2.0f, delta.y());
    Mat4 transform = editor.transform();
    rotateScaleAtOrigin(transform, 0.0, scaling, viewportPos);
    editor.setTransform(transform);
}

void RotateTool::wheel(const Vec2 &viewportPos, const Vec2 &delta, const int mode)
{
    const float rotation = -15.0f * delta.y();
    Mat4 transform = editor.transform();
    rotateScaleAtOrigin(transform, rotation, 1.0f, viewportPos);
    editor.setTransform(transform);
}

} // namespace GfxPaint
