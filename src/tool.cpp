#include "tool.h"

#include "application.h"
#include "editingcontext.h"
#include "editor.h"

namespace GfxPaint {

std::map<QString, Program *> PixelTool::formatPrograms(EditingContext &context, const Buffer::Format &bufferFormat, const bool indexed, const Buffer::Format &paletteFormat) const
{
    return {{"render", new PixelLineProgram(bufferFormat, indexed, paletteFormat, context.blendMode, context.composeMode)}};
}

void PixelTool::end(EditingContext &context, const Mat4 &viewTransform)
{
    for (Node *node : context.selectedNodes()) {
        const Traversal::State &state = context.states().at(node);
        BufferNode *const bufferNode = dynamic_cast<BufferNode *>(node);
        if (bufferNode) {
            Buffer *const restoreBuffer = context.selectedNodeRestoreBuffers[node];
            ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
            bufferNode->buffer.bindFramebuffer();

            // World to buffer
            Mat4 worldToBuffer = state.transform.inverted();
            // Buffer to clip
            Mat4 bufferToClip = bufferNode->viewportTransform();

            PixelLineProgram *pixelLineProgram = static_cast<PixelLineProgram *>(context.toolProgram(bufferNode->buffer.format(), bufferNode->indexed, state.palette ? state.palette->format() : Buffer::Format(), this, "render"));
            pixelLineProgram->render(context.toolStroke.points, context.colour, worldToBuffer, bufferToClip, restoreBuffer, state.palette);
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

std::map<QString, Program *> BrushTool::formatPrograms(EditingContext &context, const Buffer::Format &bufferFormat, const bool indexed, const Buffer::Format &paletteFormat) const
{
    return {{"render", new BrushDabProgram(context.brush.dab.type, context.brush.dab.metric, bufferFormat, indexed, paletteFormat, context.blendMode, context.composeMode)}};
}

void BrushTool::end(EditingContext &context, const Mat4 &viewTransform)
{
    for (Node *node : context.selectedNodes()) {
        const Traversal::State &state = context.states().at(node);
        BufferNode *const bufferNode = dynamic_cast<BufferNode *>(node);
        if (bufferNode) {
            Buffer *const restoreBuffer = context.selectedNodeRestoreBuffers[node];
            ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
            bufferNode->buffer.bindFramebuffer();

            // World to buffer
            Mat4 worldToBuffer = state.transform.inverted();
            // Buffer to clip
            Mat4 bufferToClip = bufferNode->viewportTransform();

            BrushDabProgram *brushDabProgram = static_cast<BrushDabProgram *>(context.toolProgram(bufferNode->buffer.format(), bufferNode->indexed, state.palette ? state.palette->format() : Buffer::Format(), this, "render"));
            brushDabProgram->render(context.toolStroke.points, context.brush.dab, context.colour, worldToBuffer, bufferToClip, restoreBuffer, state.palette);

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

void PrimitiveTool::end(EditingContext &context, const Mat4 &viewTransform)
{
    update(context, viewTransform);
    for (Node *node : context.selectedNodes()) {
        const Traversal::State &state = context.states().at(node);
        BufferNode *const bufferNode = dynamic_cast<BufferNode *>(node);
        if (bufferNode) {
            Buffer *const restoreBuffer = context.selectedNodeRestoreBuffers[node];
            ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
            bufferNode->buffer.bindFramebuffer();

            //            const Mat4 toolSpaceTransform = state.transform.inverted(); // World-space to object-space
            //            const Mat4 toolSpaceTransform = Mat4(); // World-space to world-space
            //            const Mat4 toolSpaceTransform = viewTransform; // World-space to view-space
            Mat4 toolSpaceTransform = Editor::toolSpace(context, viewTransform, *bufferNode, context.toolSpace);
            BoundedPrimitiveProgram *program = dynamic_cast<BoundedPrimitiveProgram *>(context.toolProgram(bufferNode->buffer.format(), bufferNode->indexed, state.palette ? state.palette->format() : Buffer::Format(), this, "render"));
            program->render({context.toolStroke.points.front().pos, context.toolStroke.points.back().pos}, context.colour, toolSpaceTransform, bufferNode->viewportTransform() * state.transform.inverted(), restoreBuffer, state.palette);
        }
    }
}

void PrimitiveTool::onCanvasPreview(EditingContext &context, const Mat4 &viewTransform, const bool isActive)
{
    if (isActive) {
        end(context, viewTransform);
    }
}

void PrimitiveTool::onTopPreview(Editor &editor, EditingContext &context, const Mat4 &viewTransform, const bool isActive)
{
    if (isActive) {
        ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
        Model *const markerModel = qApp->renderManager.models["planeMarker"];
        VertexColourModelProgram *const markerProgram = static_cast<VertexColourModelProgram *>(qApp->renderManager.programs["marker"]);
        Mat4 markerTransform = editor.getViewportTransform();
        const Vec2 viewportPoint = viewTransform * context.toolStroke.points[0].pos;
        markerTransform.translate(viewportPoint.toVector3D());
        float markerSize = 16.0f;
        markerTransform.scale(markerSize, markerSize);
        markerProgram->render(markerModel, markerTransform, editor.getWidgetBuffer(), nullptr);
    }
}

std::map<QString, Program *> RectTool::formatPrograms(EditingContext &context, const Buffer::Format &bufferFormat, const bool indexed, const Buffer::Format &paletteFormat) const
{
    return {{"render", new RectProgram(false, bufferFormat, indexed, paletteFormat, context.blendMode, context.composeMode)}};
}

std::map<QString, Program *> FilledRectTool::formatPrograms(EditingContext &context, const Buffer::Format &bufferFormat, const bool indexed, const Buffer::Format &paletteFormat) const
{
    return {{"render", new RectProgram(true, bufferFormat, indexed, paletteFormat, context.blendMode, context.composeMode)}};
}

std::map<QString, Program *> EllipseTool::formatPrograms(EditingContext &context, const Buffer::Format &bufferFormat, const bool indexed, const Buffer::Format &paletteFormat) const
{
    return {{"render", new EllipseProgram(false, bufferFormat, indexed, paletteFormat, context.blendMode, context.composeMode)}};
}

std::map<QString, Program *> FilledEllipseTool::formatPrograms(EditingContext &context, const Buffer::Format &bufferFormat, const bool indexed, const Buffer::Format &paletteFormat) const
{
    return {{"render", new EllipseProgram(true, bufferFormat, indexed, paletteFormat, context.blendMode, context.composeMode)}};
}

std::map<QString, Program *> ContourTool::formatPrograms(EditingContext &context, const Buffer::Format &bufferFormat, const bool indexed, const Buffer::Format &paletteFormat) const
{
    return {
        {"stencil", new ContourStencilProgram(bufferFormat, indexed, paletteFormat, 0, RenderManager::composeModeDefault)},
        {"colour", new SingleColourModelProgram(bufferFormat, indexed, paletteFormat, context.blendMode, context.composeMode)},
    };
}

void ContourTool::end(EditingContext &context, const Mat4 &viewTransform)
{
    update(context, viewTransform);
    for (Node *node : context.selectedNodes()) {
        const Traversal::State &state = context.states().at(node);
        BufferNode *const bufferNode = dynamic_cast<BufferNode *>(node);
        if (bufferNode) {
            Buffer *const restoreBuffer = context.selectedNodeRestoreBuffers[node];

            ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
            bufferNode->buffer.bindFramebuffer();

            ContourStencilProgram *stencilProgram = static_cast<ContourStencilProgram *>(context.toolProgram(bufferNode->buffer.format(), bufferNode->indexed, state.palette ? state.palette->format() : Buffer::Format(), this, "stencil"));
            stencilProgram->render(context.toolStroke.points, bufferNode->viewportTransform() * state.transform.inverted(), restoreBuffer);

            const Bounds2 &bounds = context.toolStroke.bounds;
//            qDebug() << context.toolStroke.bounds;///////////////////////////
            Model model = {GL_TRIANGLE_STRIP, {2}, {
                            (float)bounds.min.x(), (float)bounds.min.y(),
                            (float)bounds.max.x(), (float)bounds.min.y(),
                            (float)bounds.min.x(), (float)bounds.max.y(),
                            (float)bounds.max.x(), (float)bounds.max.y()},
                           {{0, 1, 2, 3}}, {4}};
            SingleColourModelProgram *modelProgram = static_cast<SingleColourModelProgram *>(context.toolProgram(bufferNode->buffer.format(), bufferNode->indexed, state.palette ? state.palette->format() : Buffer::Format(), this, "colour"));
            modelProgram->render(&model, context.colour, bufferNode->viewportTransform() * state.transform.inverted(), restoreBuffer, state.palette);

            stencilProgram->postRender();
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
        VertexColourModelProgram *const markerProgram = static_cast<VertexColourModelProgram *>(qApp->renderManager.programs["marker"]);
        Mat4 markerTransform = editor.getViewportTransform();
        const Vec2 viewportPoint = viewTransform * context.toolStroke.points[0].pos;
        markerTransform.translate(viewportPoint.toVector3D());
        float markerSize = 16.0f;
        markerTransform.scale(markerSize, markerSize);
        markerProgram->render(markerModel, markerTransform, editor.getWidgetBuffer(), nullptr);
    }
}

std::map<QString, Program *> PourFillTool::formatPrograms(EditingContext &context, const Buffer::Format &bufferFormat, const bool indexed, const Buffer::Format &paletteFormat) const
{
    return {
        {"stencil", new ContourStencilProgram(bufferFormat, indexed, paletteFormat, 0, RenderManager::composeModeDefault)},
        {"colour", new SingleColourModelProgram(bufferFormat, indexed, paletteFormat, context.blendMode, context.composeMode)},
    };
}

void PourFillTool::end(EditingContext &context, const Mat4 &viewTransform)
{
    update(context, viewTransform);
    for (Node *node : context.selectedNodes()) {
        const Traversal::State &state = context.states().at(node);
        BufferNode *const bufferNode = dynamic_cast<BufferNode *>(node);
        if (bufferNode) {
            Buffer *const restoreBuffer = context.selectedNodeRestoreBuffers[node];

            ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
            bufferNode->buffer.bindFramebuffer();

            ContourStencilProgram *stencilProgram = static_cast<ContourStencilProgram *>(context.toolProgram(bufferNode->buffer.format(), bufferNode->indexed, state.palette ? state.palette->format() : Buffer::Format(), this, "stencil"));
            stencilProgram->render(context.toolStroke.points, bufferNode->viewportTransform() * state.transform.inverted(), restoreBuffer);

            const Bounds2 &bounds = context.toolStroke.bounds;
            Model model = {GL_TRIANGLE_STRIP, {2}, {
                (float)bounds.min.x(), (float)bounds.min.y(),
                (float)bounds.max.x(), (float)bounds.min.y(),
                (float)bounds.min.x(), (float)bounds.max.y(),
                (float)bounds.max.x(), (float)bounds.max.y()},
            {{0, 1, 2, 3}}, {4}};
            SingleColourModelProgram *modelProgram = static_cast<SingleColourModelProgram *>(context.toolProgram(bufferNode->buffer.format(), bufferNode->indexed, state.palette ? state.palette->format() : Buffer::Format(), this, "colour"));
            modelProgram->render(&model, context.colour, bufferNode->viewportTransform() * state.transform.inverted(), restoreBuffer, state.palette);

            stencilProgram->postRender();
        }
    }
}

void PourFillTool::onCanvasPreview(EditingContext &context, const Mat4 &viewTransform, const bool isActive)
{
    if (isActive) {
        end(context, viewTransform);
    }
}

void PourFillTool::onTopPreview(Editor &editor, EditingContext &context, const Mat4 &viewTransform, const bool isActive)
{
    if (isActive) {
        ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
        Model *const markerModel = qApp->renderManager.models["planeMarker"];
        VertexColourModelProgram *const markerProgram = static_cast<VertexColourModelProgram *>(qApp->renderManager.programs["marker"]);
        Mat4 markerTransform = editor.getViewportTransform();
        const Vec2 viewportPoint = viewTransform * context.toolStroke.points[0].pos;
        markerTransform.translate(viewportPoint.toVector3D());
        float markerSize = 16.0f;
        markerTransform.scale(markerSize, markerSize);
        markerProgram->render(markerModel, markerTransform, editor.getWidgetBuffer(), nullptr);
    }
}

std::map<QString, Program *> ColourPickTool::formatPrograms(EditingContext &context, const Buffer::Format &bufferFormat, const bool indexed, const Buffer::Format &paletteFormat) const
{
    return {{"pick", new ColourPickProgram(bufferFormat, indexed, paletteFormat)}};
}

void ColourPickTool::begin(EditingContext &context, const Mat4 &viewTransform)
{
    update(context, viewTransform);
}

void ColourPickTool::update(EditingContext &context, const Mat4 &viewTransform)
{
    for (Node *node : context.selectedNodes()) {
        const Traversal::State &state = context.states().at(node);
        BufferNode *const bufferNode = dynamic_cast<BufferNode *>(node);
        if (bufferNode) {
            const Vec2 bufferPoint = state.transform.inverted() * context.toolStroke.points.back().pos;
            ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
            ColourPickProgram *colourPickProgram = static_cast<ColourPickProgram *>(context.toolProgram(bufferNode->buffer.format(), bufferNode->indexed, state.palette ? state.palette->format() : Buffer::Format(), this, "pick"));
            context.colour = colourPickProgram->pick(&bufferNode->buffer, bufferNode->indexed ? state.palette : nullptr, bufferPoint);
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
            const Traversal::State &state = context.states().at(node);
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
            const Traversal::State &state = context.states().at(node);
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
        VertexColourModelProgram *const markerProgram = static_cast<VertexColourModelProgram *>(qApp->renderManager.programs["marker"]);
        Mat4 markerTransform = editor.getViewportTransform();
        float markerSize = 16.0f;
        markerTransform.scale(markerSize, markerSize);
        markerProgram->render(markerModel, markerTransform, editor.getWidgetBuffer(), nullptr);
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
