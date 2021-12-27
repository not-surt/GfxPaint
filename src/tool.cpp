#include "tool.h"

#include "editor.h"

namespace GfxPaint {

void StrokeTool::begin(EditingContext &context, const Mat4 &viewTransform) {
    strokeOffset = 0.0;
}

void StrokeTool::update(EditingContext &context, const Mat4 &viewTransform) {
    const int lastPointIndex = context.toolStroke.points.size() - 1;
    const Stroke::Point &prevWorldPoint = (context.toolStroke.points.count() == 1 ? context.toolStroke.points[lastPointIndex] : context.toolStroke.points[lastPointIndex - 1]);
    const Stroke::Point &worldPoint = context.toolStroke.points[lastPointIndex];
    for (Node *node : context.selectedNodes()) {
        const Traversal::State &state = context.states().value(node);
        BufferNode *const bufferNode = dynamic_cast<BufferNode *>(node);
        const EditingContext::BufferNodeContext *const bufferNodeContext = context.bufferNodeContext(node);
        if (bufferNode) {
            ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
            bufferNode->buffer.bindFramebuffer();
            const Brush &brush = context.brush();
            Stroke dabStroke;
            strokeOffset = Editor::strokeSegmentDabs(prevWorldPoint, worldPoint, brush.dab.size, brush.stroke.absoluteSpacing, brush.stroke.proportionalSpacing, strokeOffset, dabStroke);
            // TODO: instancing?
            for (const auto &point : dabStroke.points) {
                const Vec2 snappedPoint = Editor::pixelSnap(context, point.pos);
                Brush::Dab dab(brush.dab);
                dab.size *= point.pressure;
                dab.angle += point.quaternion.toEulerAngles().z();
                Editor::drawDab(context, viewTransform, dab, context.colour(), *bufferNode, snappedPoint);
            }

            // Draw to stroke buffer
//            bufferNodeContext->strokeBuffer->bindFramebuffer();
//            for (auto pos : points) {
//                const Vector2D snappedPoint = Editor::pixelSnap(context, pos);
//                Editor::drawDab(context, viewTransform, dab, context.colour(), *bufferNode, snappedPoint);
//            }
        }
    }
}

void StrokeTool::end(EditingContext &context, const Mat4 &viewTransform) {
    if (context.toolStroke.points.count() == 1) update(context, viewTransform);

    // Clear stroke buffer
    for (Node *node : context.selectedNodes()) {
        const Traversal::State &state = context.states().value(node);
        BufferNode *const bufferNode = dynamic_cast<BufferNode *>(node);
        const EditingContext::BufferNodeContext *const bufferNodeContext = context.bufferNodeContext(node);
        if (bufferNode) {
//            bufferNodeContext->strokeBuffer->clear();
        }
    }
}

void StrokeTool::onCanvasPreview(EditingContext &context, const Mat4 &viewTransform, const bool isActive)
{
    auto saveOffset = strokeOffset;

    begin(context, viewTransform);
    end(context, viewTransform);

    strokeOffset = saveOffset;
}

void RectTool::end(EditingContext &context, const Mat4 &viewTransform)
{
    const PrimitiveTool::Mode primitiveMode = static_cast<PrimitiveTool::Mode>(context.toolMode);

    update(context, viewTransform);
    for (Node *node : context.selectedNodes()) {
        const Traversal::State &state = context.states().value(node);
        BufferNode *const bufferNode = dynamic_cast<BufferNode *>(node);
        const EditingContext::BufferNodeContext *const bufferNodeContext = context.bufferNodeContext(node);
        if (bufferNode) {
            ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
            bufferNode->buffer.bindFramebuffer();

//            const Mat4 toolSpaceTransform = state.transform.inverted(); // World-space to object-space
//            const Mat4 toolSpaceTransform = Mat4(); // World-space to world-space
//            const Mat4 toolSpaceTransform = viewTransform; // World-space to view-space
            Mat4 toolSpaceTransform = Editor::toolSpace(context, viewTransform, *bufferNode, context.space());
            RectProgram *const program = new RectProgram(primitiveMode == PrimitiveTool::Mode::Filled, bufferNode->buffer.format(), state.palette != nullptr, state.palette != nullptr ? state.palette->format() : Buffer::Format(), context.blendMode(), context.composeMode());
            program->render({context.toolStroke.points.front().pos, context.toolStroke.points.back().pos}, context.colour(), toolSpaceTransform, bufferNode->viewportTransform() * state.transform.inverted(), &bufferNode->buffer, state.palette);
            delete program;
        }
    }
}

void RectTool::onCanvasPreview(EditingContext &context, const Mat4 &viewTransform, const bool isActive)
{
    if (isActive) {
        end(context, viewTransform);
    }
}

void RectTool::onTopPreview(Editor &editor, EditingContext &context, const Mat4 &viewTransform, const bool isActive)
{
    if (isActive) {
        ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
        Model *const markerModel = qApp->renderManager.models["planeMarker"];
        VertexColourModelProgram *const markerProgram = new VertexColourModelProgram(RenderedWidget::format, false, Buffer::Format(), 0, RenderManager::composeModeDefault);
        Mat4 markerTransform = editor.getViewportTransform();
        const Vec2 viewportPoint = viewTransform * context.toolStroke.points[0].pos;
        markerTransform.translate(viewportPoint.toVector3D());
        float markerSize = 16.0f;
        markerTransform.scale(markerSize, markerSize);
        markerProgram->render(markerModel, markerTransform, editor.getWidgetBuffer(), nullptr);
        delete markerProgram;
    }
}

void EllipseTool::end(EditingContext &context, const Mat4 &viewTransform)
{
    const PrimitiveTool::Mode primitiveMode = static_cast<PrimitiveTool::Mode>(context.toolMode);

    update(context, viewTransform);
    for (Node *node : context.selectedNodes()) {
        const Traversal::State &state = context.states().value(node);
        BufferNode *const bufferNode = dynamic_cast<BufferNode *>(node);
        const EditingContext::BufferNodeContext *const bufferNodeContext = context.bufferNodeContext(node);
        if (bufferNode) {
            ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
            bufferNode->buffer.bindFramebuffer();

            Mat4 toolSpaceTransform = Editor::toolSpace(context, viewTransform, *bufferNode, context.space());
            EllipseProgram *const program = new EllipseProgram(primitiveMode == PrimitiveTool::Mode::Filled, bufferNode->buffer.format(), state.palette != nullptr, state.palette != nullptr ? state.palette->format() : Buffer::Format(), context.blendMode(), context.composeMode());
            program->render({context.toolStroke.points.front().pos, context.toolStroke.points.back().pos}, context.colour(), toolSpaceTransform, bufferNode->viewportTransform() * state.transform.inverted(), &bufferNode->buffer, state.palette);
            delete program;
        }
    }
}

void EllipseTool::onCanvasPreview(EditingContext &context, const Mat4 &viewTransform, const bool isActive)
{
    if (isActive) {
        end(context, viewTransform);
    }
}

void EllipseTool::onTopPreview(Editor &editor, EditingContext &context, const Mat4 &viewTransform, const bool isActive)
{
    if (isActive) {
        ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
        Model *const markerModel = qApp->renderManager.models["planeMarker"];
        VertexColourModelProgram *const markerProgram = new VertexColourModelProgram(RenderedWidget::format, false, Buffer::Format(), 0, RenderManager::composeModeDefault);
        Mat4 markerTransform = editor.getViewportTransform();
        const Vec2 viewportPoint = viewTransform * context.toolStroke.points[0].pos;
        markerTransform.translate(viewportPoint.toVector3D());
        float markerSize = 16.0f;
        markerTransform.scale(markerSize, markerSize);
        markerProgram->render(markerModel, markerTransform, editor.getWidgetBuffer(), nullptr);
        delete markerProgram;
    }
}

void ContourTool::end(EditingContext &context, const Mat4 &viewTransform)
{
    update(context, viewTransform);
    for (Node *node : context.selectedNodes()) {
        const Traversal::State &state = context.states().value(node);
        BufferNode *const bufferNode = dynamic_cast<BufferNode *>(node);
        const EditingContext::BufferNodeContext *const bufferNodeContext = context.bufferNodeContext(node);
        if (bufferNode) {
            Vec2 boundsMin = Vec2(std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity());
            Vec2 boundsMax = Vec2(-std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity());
            std::vector<Vec2> points;
            auto iterator = context.toolStroke.points.begin();
            while (iterator != context.toolStroke.points.end()) {
                boundsMin.setX(std::min(boundsMin.x(), iterator->pos.x()));
                boundsMin.setY(std::min(boundsMin.y(), iterator->pos.y()));
                boundsMax.setX(std::max(boundsMax.x(), iterator->pos.x()));
                boundsMax.setY(std::max(boundsMax.y(), iterator->pos.y()));
                points.push_back(iterator->pos);
                ++iterator;
            }

            ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
            bufferNode->buffer.bindFramebuffer();

            ContourStencilProgram *const stencilProgram = new ContourStencilProgram(bufferNode->buffer.format(), state.palette != nullptr, state.palette != nullptr ? state.palette->format() : Buffer::Format(), 0, RenderManager::composeModeDefault);
            stencilProgram->render(points, context.colour(), bufferNode->viewportTransform() * state.transform.inverted(), &bufferNode->buffer, state.palette);

            Model *const model = new Model(GL_TRIANGLE_STRIP, {2}, {
                                                                       boundsMin.x(), boundsMin.y(),
                                                                       boundsMax.x(), boundsMin.y(),
                                                                       boundsMin.x(), boundsMax.y(),
                                                                       boundsMax.x(), boundsMax.y()},
                                           {{0, 1, 2, 3}}, {4});
            SingleColourModelProgram *const modelProgram = new SingleColourModelProgram(bufferNode->buffer.format(), state.palette != nullptr, state.palette != nullptr ? state.palette->format() : Buffer::Format(), context.blendMode(), context.composeMode());
            modelProgram->render(model, context.colour(), bufferNode->viewportTransform() * state.transform.inverted(), &bufferNode->buffer, state.palette);
            delete modelProgram;

            stencilProgram->postRender();
            delete stencilProgram;
        }
    }
}

void ContourTool::onCanvasPreview(EditingContext &context, const Mat4 &viewTransform, const bool isActive)
{
    if (isActive) {
        end(context, viewTransform);
    }
}

void ContourTool::onTopPreview(Editor &editor, EditingContext &context, const Mat4 &viewTransform, const bool isActive)
{
    if (isActive) {
        ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
        Model *const markerModel = qApp->renderManager.models["planeMarker"];
        VertexColourModelProgram *const markerProgram = new VertexColourModelProgram(RenderedWidget::format, false, Buffer::Format(), 0, RenderManager::composeModeDefault);
        Mat4 markerTransform = editor.getViewportTransform();
        const Vec2 viewportPoint = viewTransform * context.toolStroke.points[0].pos;
        markerTransform.translate(viewportPoint.toVector3D());
        float markerSize = 16.0f;
        markerTransform.scale(markerSize, markerSize);
        markerProgram->render(markerModel, markerTransform, editor.getWidgetBuffer(), nullptr);
        delete markerProgram;
    }
}

void ColourPickTool::begin(EditingContext &context, const Mat4 &viewTransform)
{
    update(context, viewTransform);
}

void ColourPickTool::update(EditingContext &context, const Mat4 &viewTransform)
{
    for (Node *node : context.selectedNodes()) {
        const Traversal::State &state = context.states().value(node);
        EditingContext::BufferNodeContext *const bufferNodeContext = context.bufferNodeContext(node);
        BufferNode *const bufferNode = dynamic_cast<BufferNode *>(node);
        if (bufferNode) {
            const Vec2 bufferPoint = state.transform.inverted() * context.toolStroke.points.last().pos;
            ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
            context.setColour(bufferNodeContext->colourPickProgram->pick(&bufferNode->buffer, bufferNode->indexed ? state.palette : nullptr, bufferPoint));
        }
    }
}

bool TransformTool::isUndoable(EditingContext &context) const
{
    return context.transformTarget != EditingContext::TransformTarget::View;
}

void TransformTargetOverrideTool::begin(EditingContext &context, const Mat4 &viewTransform)
{
    oldTransformMode = static_cast<int>(context.transformTarget);
    context.transformTarget = static_cast<EditingContext::TransformTarget>(context.toolMode);
}

void TransformTargetOverrideTool::end(EditingContext &context, const Mat4 &viewTransform)
{
    context.transformTarget = static_cast<EditingContext::TransformTarget>(oldTransformMode);
}

void PanTool::begin(EditingContext &context, const Mat4 &viewTransform)
{
    const Vec2 viewportPos = viewTransform * context.toolStroke.points.last().pos;
    oldViewportPos = viewportPos;
}

void PanTool::update(EditingContext &context, const Mat4 &viewTransform)
{
    const Vec2 viewportPos = viewTransform * context.toolStroke.points.last().pos;
    const Vec2 translation = viewportPos - oldViewportPos;
    Mat4 transform;
    transform.translate(translation);
    if (context.transformTarget == EditingContext::TransformTarget::View) {
        editor.setTransform(transform * editor.transform());
    }
    else {
        for (Node *node : context.selectedNodes()) {
            const Traversal::State &state = context.states().value(node);
            if (context.transformTarget == EditingContext::TransformTarget::Object) {
                SpatialNode *const spatialNode = dynamic_cast<SpatialNode *>(node);
                if (spatialNode) {
                    spatialNode->setTransform(state.parentTransform.inverted() * (viewTransform.inverted() * transform * viewTransform) * state.parentTransform * spatialNode->transform());
                }
            }
            else if (context.transformTarget == EditingContext::TransformTarget::Brush) {

            }
        }
    }
    oldViewportPos = viewportPos;
}

void RotoZoomTool::begin(EditingContext &context, const Mat4 &viewTransform)
{
    const Vec2 viewportPos = viewTransform * context.toolStroke.points.last().pos;
    oldViewportPos = viewportPos;
}

void RotoZoomTool::update(EditingContext &context, const Mat4 &viewTransform)
{
    const Vec2 viewportPos = viewTransform * context.toolStroke.points.last().pos;
    const Mode toolMode = static_cast<Mode>(context.toolMode);
    const bool rotate = (toolMode == Mode::RotoZoom || toolMode == Mode::Rotate);
    const bool zoom = (toolMode == Mode::RotoZoom || toolMode == Mode::Zoom);
    const Mat4 transform = transformPointToPoint(Vec2(0.0f, 0.0f), oldViewportPos, viewportPos, rotate, zoom);
    if (context.transformTarget == EditingContext::TransformTarget::View) {
        editor.setTransform(transform * editor.transform());
    }
    else {
        for (Node *node : context.selectedNodes()) {
            const Traversal::State &state = context.states().value(node);
            if (context.transformTarget == EditingContext::TransformTarget::Object) {
                SpatialNode *const spatialNode = dynamic_cast<SpatialNode *>(node);
                if (spatialNode) {
                    spatialNode->setTransform(state.parentTransform.inverted() * (viewTransform.inverted() * transform * viewTransform) * state.parentTransform * spatialNode->transform());
                }
            }
            else if (context.transformTarget == EditingContext::TransformTarget::Brush) {

            }
        }
    }
    oldViewportPos = viewportPos;
}

void RotoZoomTool::onTopPreview(Editor &editor, EditingContext &context, const Mat4 &viewTransform, const bool isActive)
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

void WheelZoomTool::wheel(EditingContext &context, const Mat4 &viewTransform, const Vec2 &delta)
{
    const Vec2 viewportPos = editor.transform() * context.toolStroke.points.last().pos;
    const float scaling = std::pow(2.0f, delta.y());
    Mat4 transform = editor.transform();
    rotateScaleAtOrigin(transform, 0.0, scaling, viewportPos);
    editor.setTransform(transform);
}

void WheelRotateTool::wheel(EditingContext &context, const Mat4 &viewTransform, const Vec2 &delta)
{
    const Vec2 viewportPos = editor.transform() * context.toolStroke.points.last().pos;
    const float rotation = -15.0f * delta.y();
    Mat4 transform = editor.transform();
    rotateScaleAtOrigin(transform, rotation, 1.0f, viewportPos);
    editor.setTransform(transform);
}

} // namespace GfxPaint
