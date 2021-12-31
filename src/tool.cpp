#include "tool.h"

#include "application.h"
#include "editingcontext.h"
#include "editor.h"

namespace GfxPaint {

std::map<std::string, Program *> PixelTool::nodePrograms(EditingContext &context, Node *const node) const
{
    std::map<std::string, Program *> programs;
    const Traversal::State &state = context.states().value(node);
    BufferNode *const bufferNode = dynamic_cast<BufferNode *>(node);
    if (bufferNode) {
        programs["render"] = new PixelLineProgram(bufferNode->buffer.format(), state.palette != nullptr, state.palette != nullptr ? state.palette->format() : Buffer::Format(), context.blendMode(), context.composeMode());
    }
    return programs;
}

void PixelTool::end(EditingContext &context, const Mat4 &viewTransform)
{
    for (Node *node : context.selectedNodes()) {
        const Traversal::State &state = context.states().value(node);
        BufferNode *const bufferNode = dynamic_cast<BufferNode *>(node);
        const EditingContext::NodeEditingContext *const bufferNodeContext = context.nodeEditingContext(node);
        if (bufferNode) {
            ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
            bufferNode->buffer.bindFramebuffer();

            // World to buffer
            Mat4 worldToBuffer = state.transform.inverted();
            // Buffer to clip
            Mat4 bufferToClip = bufferNode->viewportTransform();

//            PixelLineProgram *pixelLineProgram = new PixelLineProgram(bufferNode->buffer.format(), state.palette != nullptr, state.palette != nullptr ? state.palette->format() : Buffer::Format(), context.blendMode(), context.composeMode());
            PixelLineProgram *pixelLineProgram = bufferNodeContext->pixelLineProgram;
            pixelLineProgram->render(context.toolStroke.points, context.colour(), worldToBuffer, bufferToClip, bufferNodeContext->restoreBuffer, state.palette);
        }
    }
}

void PixelTool::onCanvasPreview(EditingContext &context, const Mat4 &viewTransform, const bool isActive)
{
    if (isActive) {
        end(context, viewTransform);
    }
    else {
        begin(context, viewTransform);
        end(context, viewTransform);
    }
}

void BrushTool::end(EditingContext &context, const Mat4 &viewTransform)
{
    for (Node *node : context.selectedNodes()) {
        const Traversal::State &state = context.states().value(node);
        BufferNode *const bufferNode = dynamic_cast<BufferNode *>(node);
        const EditingContext::NodeEditingContext *const bufferNodeContext = context.nodeEditingContext(node);
        if (bufferNode) {
            ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
            bufferNode->buffer.bindFramebuffer();

            // World to buffer
            Mat4 worldToBuffer = state.transform.inverted();
            // Buffer to clip
            Mat4 bufferToClip = bufferNode->viewportTransform();

//            BrushDabProgram *brushDabProgram = new BrushDabProgram(context.brush().dab.type, context.brush().dab.metric, bufferNode->buffer.format(), state.palette != nullptr, state.palette != nullptr ? state.palette->format() : Buffer::Format(), context.blendMode(), context.composeMode());
            BrushDabProgram *brushDabProgram = bufferNodeContext->brushDabProgram;
            brushDabProgram->render(context.toolStroke.points, context.brush().dab, context.colour(), worldToBuffer, bufferToClip, bufferNodeContext->restoreBuffer, state.palette);

//            const QRectF lastSegmentBounds;
//            const Brush &brush = context.brush();
//            // TODO: instancing?
//            float strokeOffset = 0.0;
//            for (std::size_t i = 1; i < strokePoints.size(); ++i) {
//                const QRectF segmentBounds;
//                if (segmentBounds.intersects(lastSegmentBounds)) {

//                }
//                Stroke dabStroke;
//                strokeOffset = Editor::strokeSegmentDabs(strokePoints[i - 1], strokePoints[i], brush.dab.size, brush.stroke.absoluteSpacing, brush.stroke.proportionalSpacing, strokeOffset, dabStroke);
//                for (const auto &dabPoint : dabStroke.points) {
//                    const Vec2 snappedPoint = Editor::pixelSnap(context, dabPoint.pos);
//                    Brush::Dab dab(brush.dab);
//                    dab.size *= dabPoint.pressure;
//                    dab.angle += dabPoint.quaternion.toEulerAngles().z();
//                    Editor::drawDab(context, viewTransform, dab, context.colour(), *bufferNode, snappedPoint);
//                }
//            }
        }
    }
}

void BrushTool::onCanvasPreview(EditingContext &context, const Mat4 &viewTransform, const bool isActive)
{
    if (isActive) {
        end(context, viewTransform);
    }
    else {
        begin(context, viewTransform);
        end(context, viewTransform);
    }
}

void RectTool::end(EditingContext &context, const Mat4 &viewTransform)
{
    const PrimitiveTool::Mode primitiveMode = static_cast<PrimitiveTool::Mode>(context.toolMode);

    update(context, viewTransform);
    for (Node *node : context.selectedNodes()) {
        const Traversal::State &state = context.states().value(node);
        BufferNode *const bufferNode = dynamic_cast<BufferNode *>(node);
        const EditingContext::NodeEditingContext *const bufferNodeContext = context.nodeEditingContext(node);
        if (bufferNode) {
            ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
            bufferNode->buffer.bindFramebuffer();

//            const Mat4 toolSpaceTransform = state.transform.inverted(); // World-space to object-space
//            const Mat4 toolSpaceTransform = Mat4(); // World-space to world-space
//            const Mat4 toolSpaceTransform = viewTransform; // World-space to view-space
            Mat4 toolSpaceTransform = Editor::toolSpace(context, viewTransform, *bufferNode, context.space());
            RectProgram *const program = new RectProgram(primitiveMode == PrimitiveTool::Mode::Filled, bufferNode->buffer.format(), state.palette != nullptr, state.palette != nullptr ? state.palette->format() : Buffer::Format(), context.blendMode(), context.composeMode());
            program->render({context.toolStroke.points.front().pos, context.toolStroke.points.back().pos}, context.colour(), toolSpaceTransform, bufferNode->viewportTransform() * state.transform.inverted(), bufferNodeContext->restoreBuffer, state.palette);
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
        const EditingContext::NodeEditingContext *const bufferNodeContext = context.nodeEditingContext(node);
        if (bufferNode) {
            ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
            bufferNode->buffer.bindFramebuffer();

            Mat4 toolSpaceTransform = Editor::toolSpace(context, viewTransform, *bufferNode, context.space());
            EllipseProgram *const program = new EllipseProgram(primitiveMode == PrimitiveTool::Mode::Filled, bufferNode->buffer.format(), state.palette != nullptr, state.palette != nullptr ? state.palette->format() : Buffer::Format(), context.blendMode(), context.composeMode());
            program->render({context.toolStroke.points.front().pos, context.toolStroke.points.back().pos}, context.colour(), toolSpaceTransform, bufferNode->viewportTransform() * state.transform.inverted(), bufferNodeContext->restoreBuffer, state.palette);
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
        const EditingContext::NodeEditingContext *const bufferNodeContext = context.nodeEditingContext(node);
        if (bufferNode) {
            Vec2 boundsMin = Vec2(std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity());
            Vec2 boundsMax = Vec2(-std::numeric_limits<float>::infinity(), -std::numeric_limits<float>::infinity());
//            std::vector<Vec2> points;
            auto iterator = context.toolStroke.points.begin();
            while (iterator != context.toolStroke.points.end()) {
                boundsMin.setX(std::min(boundsMin.x(), iterator->pos.x()));
                boundsMin.setY(std::min(boundsMin.y(), iterator->pos.y()));
                boundsMax.setX(std::max(boundsMax.x(), iterator->pos.x()));
                boundsMax.setY(std::max(boundsMax.y(), iterator->pos.y()));
//                points.push_back(iterator->pos);
                ++iterator;
            }

            ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
            bufferNode->buffer.bindFramebuffer();

            ContourStencilProgram *const stencilProgram = new ContourStencilProgram(bufferNode->buffer.format(), state.palette != nullptr, state.palette != nullptr ? state.palette->format() : Buffer::Format(), 0, RenderManager::composeModeDefault);
            stencilProgram->render(context.toolStroke.points, context.colour(), bufferNode->viewportTransform() * state.transform.inverted(), bufferNodeContext->restoreBuffer, state.palette);

            Model *const model = new Model(GL_TRIANGLE_STRIP, {2}, {
                                                                       boundsMin.x(), boundsMin.y(),
                                                                       boundsMax.x(), boundsMin.y(),
                                                                       boundsMin.x(), boundsMax.y(),
                                                                       boundsMax.x(), boundsMax.y()},
                                           {{0, 1, 2, 3}}, {4});
            SingleColourModelProgram *const modelProgram = new SingleColourModelProgram(bufferNode->buffer.format(), state.palette != nullptr, state.palette != nullptr ? state.palette->format() : Buffer::Format(), context.blendMode(), context.composeMode());
            modelProgram->render(model, context.colour(), bufferNode->viewportTransform() * state.transform.inverted(), bufferNodeContext->restoreBuffer, state.palette);
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
        EditingContext::NodeEditingContext *const bufferNodeContext = context.nodeEditingContext(node);
        BufferNode *const bufferNode = dynamic_cast<BufferNode *>(node);
        if (bufferNode) {
            const Vec2 bufferPoint = state.transform.inverted() * context.toolStroke.points.back().pos;
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
    const Vec2 viewportPos = viewTransform * context.toolStroke.points.back().pos;
    oldViewportPos = viewportPos;
}

void PanTool::update(EditingContext &context, const Mat4 &viewTransform)
{
    const Vec2 viewportPos = viewTransform * context.toolStroke.points.back().pos;
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
    const Vec2 viewportPos = viewTransform * context.toolStroke.points.back().pos;
    oldViewportPos = viewportPos;
}

void RotoZoomTool::update(EditingContext &context, const Mat4 &viewTransform)
{
    const Vec2 viewportPos = viewTransform * context.toolStroke.points.back().pos;
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
    const Vec2 viewportPos = editor.transform() * context.toolStroke.points.back().pos;
    const float scaling = std::pow(2.0f, delta.y());
    Mat4 transform = editor.transform();
    rotateScaleAtOrigin(transform, 0.0, scaling, viewportPos);
    editor.setTransform(transform);
}

void WheelRotateTool::wheel(EditingContext &context, const Mat4 &viewTransform, const Vec2 &delta)
{
    const Vec2 viewportPos = editor.transform() * context.toolStroke.points.back().pos;
    const float rotation = -15.0f * delta.y();
    Mat4 transform = editor.transform();
    rotateScaleAtOrigin(transform, rotation, 1.0f, viewportPos);
    editor.setTransform(transform);
}

} // namespace GfxPaint
