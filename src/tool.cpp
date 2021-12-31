#include "tool.h"

#include "application.h"
#include "editingcontext.h"
#include "editor.h"

namespace GfxPaint {

std::map<QString, Program *> PixelTool::nodePrograms(EditingContext &context, Node *const node, const Traversal::State &state) const
{
    BufferNode *const bufferNode = dynamic_cast<BufferNode *>(node);
    if (bufferNode) {
        return {{"render", new PixelLineProgram(bufferNode->buffer.format(), state.palette != nullptr, state.palette != nullptr ? state.palette->format() : Buffer::Format(), context.blendMode, context.composeMode)}};
    }
    else return {};
}

void PixelTool::end(EditingContext &context, const Mat4 &viewTransform)
{
    for (Node *node : context.selectedNodes()) {
        auto &programs = context.selectedNodePrograms[node];
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

            PixelLineProgram *pixelLineProgram = static_cast<PixelLineProgram *>(context.selectedNodePrograms[node]["render"]);
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

std::map<QString, Program *> BrushTool::nodePrograms(EditingContext &context, Node * const node, const Traversal::State &state) const
{
    BufferNode *const bufferNode = dynamic_cast<BufferNode *>(node);
    if (bufferNode) {
        return {{"render", new BrushDabProgram(context.brush.dab.type, context.brush.dab.metric, bufferNode->buffer.format(), state.palette != nullptr, state.palette != nullptr ? state.palette->format() : Buffer::Format(), context.blendMode, context.composeMode)}};
    }
    else return {};
}

void BrushTool::end(EditingContext &context, const Mat4 &viewTransform)
{
    for (Node *node : context.selectedNodes()) {
        auto &programs = context.selectedNodePrograms[node];
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

            BrushDabProgram *brushDabProgram = static_cast<BrushDabProgram *>(programs["render"]);
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

std::map<QString, Program *> RectTool::nodePrograms(EditingContext &context, Node * const node, const Traversal::State &state) const
{
    const PrimitiveTool::Mode primitiveMode = static_cast<PrimitiveTool::Mode>(context.toolMode);
    BufferNode *const bufferNode = dynamic_cast<BufferNode *>(node);
    if (bufferNode) {
        return {
            {"render", new RectProgram(primitiveMode == PrimitiveTool::Mode::Filled, bufferNode->buffer.format(), state.palette != nullptr, state.palette != nullptr ? state.palette->format() : Buffer::Format(), context.blendMode, context.composeMode)},
        };
    }
    else return {};
}

void RectTool::end(EditingContext &context, const Mat4 &viewTransform)
{
    update(context, viewTransform);
    for (Node *node : context.selectedNodes()) {
        auto &programs = context.selectedNodePrograms[node];
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
            RectProgram *program = static_cast<RectProgram *>(programs["render"]);
            program->render({context.toolStroke.points.front().pos, context.toolStroke.points.back().pos}, context.colour, toolSpaceTransform, bufferNode->viewportTransform() * state.transform.inverted(), restoreBuffer, state.palette);
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
        VertexColourModelProgram *const markerProgram = static_cast<VertexColourModelProgram *>(qApp->renderManager.programs["marker"]);
        Mat4 markerTransform = editor.getViewportTransform();
        const Vec2 viewportPoint = viewTransform * context.toolStroke.points[0].pos;
        markerTransform.translate(viewportPoint.toVector3D());
        float markerSize = 16.0f;
        markerTransform.scale(markerSize, markerSize);
        markerProgram->render(markerModel, markerTransform, editor.getWidgetBuffer(), nullptr);
    }
}

std::map<QString, Program *> EllipseTool::nodePrograms(EditingContext &context, Node * const node, const Traversal::State &state) const
{
    const PrimitiveTool::Mode primitiveMode = static_cast<PrimitiveTool::Mode>(context.toolMode);
    BufferNode *const bufferNode = dynamic_cast<BufferNode *>(node);
    if (bufferNode) {
        return {
            {"render", new EllipseProgram(primitiveMode == PrimitiveTool::Mode::Filled, bufferNode->buffer.format(), state.palette != nullptr, state.palette != nullptr ? state.palette->format() : Buffer::Format(), context.blendMode, context.composeMode)},
        };
    }
    else return {};
}

void EllipseTool::end(EditingContext &context, const Mat4 &viewTransform)
{
    update(context, viewTransform);
    for (Node *node : context.selectedNodes()) {
        auto &programs = context.selectedNodePrograms[node];
        const Traversal::State &state = context.states().at(node);
        BufferNode *const bufferNode = dynamic_cast<BufferNode *>(node);
        if (bufferNode) {
            Buffer *const restoreBuffer = context.selectedNodeRestoreBuffers[node];
            ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
            bufferNode->buffer.bindFramebuffer();

            Mat4 toolSpaceTransform = Editor::toolSpace(context, viewTransform, *bufferNode, context.toolSpace);
            EllipseProgram *program = static_cast<EllipseProgram *>(programs["render"]);
            program->render({context.toolStroke.points.front().pos, context.toolStroke.points.back().pos}, context.colour, toolSpaceTransform, bufferNode->viewportTransform() * state.transform.inverted(), restoreBuffer, state.palette);
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
        VertexColourModelProgram *const markerProgram = static_cast<VertexColourModelProgram *>(qApp->renderManager.programs["marker"]);
        Mat4 markerTransform = editor.getViewportTransform();
        const Vec2 viewportPoint = viewTransform * context.toolStroke.points[0].pos;
        markerTransform.translate(viewportPoint.toVector3D());
        float markerSize = 16.0f;
        markerTransform.scale(markerSize, markerSize);
        markerProgram->render(markerModel, markerTransform, editor.getWidgetBuffer(), nullptr);
    }
}

std::map<QString, Program *> ContourTool::nodePrograms(EditingContext &context, Node * const node, const Traversal::State &state) const
{
    BufferNode *const bufferNode = dynamic_cast<BufferNode *>(node);
    if (bufferNode) {
        return {
            {"stencil", new ContourStencilProgram(bufferNode->buffer.format(), state.palette != nullptr, state.palette != nullptr ? state.palette->format() : Buffer::Format(), 0, RenderManager::composeModeDefault)},
            {"colour", new SingleColourModelProgram(bufferNode->buffer.format(), state.palette != nullptr, state.palette != nullptr ? state.palette->format() : Buffer::Format(), context.blendMode, context.composeMode)},
        };
    }
    else return {};
}

void ContourTool::end(EditingContext &context, const Mat4 &viewTransform)
{
    update(context, viewTransform);
    for (Node *node : context.selectedNodes()) {
        auto &programs = context.selectedNodePrograms[node];
        const Traversal::State &state = context.states().at(node);
        BufferNode *const bufferNode = dynamic_cast<BufferNode *>(node);
        if (bufferNode) {
            Buffer *const restoreBuffer = context.selectedNodeRestoreBuffers[node];
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

            ContourStencilProgram *stencilProgram = static_cast<ContourStencilProgram *>(programs["stencil"]);
            stencilProgram->render(context.toolStroke.points, bufferNode->viewportTransform() * state.transform.inverted(), restoreBuffer);

            Model model = {GL_TRIANGLE_STRIP, {2}, {
                            boundsMin.x(), boundsMin.y(),
                            boundsMax.x(), boundsMin.y(),
                            boundsMin.x(), boundsMax.y(),
                            boundsMax.x(), boundsMax.y()},
                           {{0, 1, 2, 3}}, {4}};
            SingleColourModelProgram *modelProgram = static_cast<SingleColourModelProgram *>(programs["colour"]);
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

std::map<QString, Program *> ColourPickTool::nodePrograms(EditingContext &context, Node * const node, const Traversal::State &state) const
{
    BufferNode *const bufferNode = dynamic_cast<BufferNode *>(node);
    if (bufferNode) {
        return {{"pick", new ColourPickProgram(bufferNode->buffer.format(), bufferNode->indexed, state.palette ? state.palette->format() : Buffer::Format())}};
    }
    else return {};
}

void ColourPickTool::begin(EditingContext &context, const Mat4 &viewTransform)
{
    update(context, viewTransform);
}

void ColourPickTool::update(EditingContext &context, const Mat4 &viewTransform)
{
    for (Node *node : context.selectedNodes()) {
        auto &programs = context.selectedNodePrograms[node];
        const Traversal::State &state = context.states().at(node);
        BufferNode *const bufferNode = dynamic_cast<BufferNode *>(node);
        if (bufferNode) {
            const Vec2 bufferPoint = state.transform.inverted() * context.toolStroke.points.back().pos;
            ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
            ColourPickProgram *colourPickProgram = static_cast<ColourPickProgram *>(programs["pick"]);
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
