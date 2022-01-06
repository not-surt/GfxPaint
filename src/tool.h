#ifndef TOOL_H
#define TOOL_H

#include <QList>
#include <deque>

#include "types.h"
#include "stroke.h"
#include "program.h"
#include "scene.h"

namespace GfxPaint {

class Editor;
class EditingContext;
class Node;

class Tool {
public:
    Tool() {}
    virtual std::map<QString, Program *> formatPrograms(EditingContext &context, const Buffer::Format &bufferFormat, const bool indexed, const Buffer::Format &paletteFormat) const { return {}; }
    virtual void begin(EditingContext &context, const Mat4 &viewTransform) {}
    virtual void update(EditingContext &context, const Mat4 &viewTransform) {}
    virtual void end(EditingContext &context, const Mat4 &viewTransform) {}
    virtual void onCanvasPreview(EditingContext &context, const Mat4 &viewTransform, const bool isActive) {}
    virtual void onTopPreview(Editor &editor, EditingContext &context, const Mat4 &viewTransform, const bool isActive) {}
    virtual void abort(EditingContext &context, const Mat4 &viewTransform) {}
    virtual void wheel(EditingContext &context, const Mat4 &viewTransform, const Vec2 &delta) {}
    virtual bool isExclusive() const { return true; }
    virtual bool updatesContext() const { return false; }
    virtual bool updatesViewTransform() const { return false; }
    virtual bool isUndoable(EditingContext &context) const { return true; }
};

class PixelTool : public Tool {
public:
    using Tool::Tool;
    virtual std::map<QString, Program *> formatPrograms(EditingContext &context, const Buffer::Format &bufferFormat, const bool indexed, const Buffer::Format &paletteFormat) const override;
    virtual void end(EditingContext &context, const Mat4 &viewTransform) override;
    virtual void onCanvasPreview(EditingContext &context, const Mat4 &viewTransform, const bool isActive) override;
};

class BrushTool : public Tool {
public:
    using Tool::Tool;
    virtual std::map<QString, Program *> formatPrograms(EditingContext &context, const Buffer::Format &bufferFormat, const bool indexed, const Buffer::Format &paletteFormat) const override;
    virtual void end(EditingContext &context, const Mat4 &viewTransform) override;
    virtual void onCanvasPreview(EditingContext &context, const Mat4 &viewTransform, const bool isActive) override;
};

class PrimitiveTool : public Tool {
public:
    enum class Mode {
        Lined,
        Filled,
    };
    enum class ModifierMode {
        Bounded,
        Centred,
        BoundedFixedAspectRatio,
        CentredFixedAspectRatio,
    };

    using Tool::Tool;
    virtual void end(EditingContext &context, const Mat4 &viewTransform) override;
    virtual void onCanvasPreview(EditingContext &context, const Mat4 &viewTransform, const bool isActive) override;
    virtual void onTopPreview(Editor &editor, EditingContext &context, const Mat4 &viewTransform, const bool isActive) override;
};

class RectTool : public PrimitiveTool {
public:
    using PrimitiveTool::PrimitiveTool;
    virtual std::map<QString, Program *> formatPrograms(EditingContext &context, const Buffer::Format &bufferFormat, const bool indexed, const Buffer::Format &paletteFormat) const override;
};

class FilledRectTool : public PrimitiveTool {
public:
    using PrimitiveTool::PrimitiveTool;
    virtual std::map<QString, Program *> formatPrograms(EditingContext &context, const Buffer::Format &bufferFormat, const bool indexed, const Buffer::Format &paletteFormat) const override;
};

class EllipseTool : public PrimitiveTool {
public:
    using PrimitiveTool::PrimitiveTool;
    virtual std::map<QString, Program *> formatPrograms(EditingContext &context, const Buffer::Format &bufferFormat, const bool indexed, const Buffer::Format &paletteFormat) const override;
};

class FilledEllipseTool : public PrimitiveTool {
public:
    using PrimitiveTool::PrimitiveTool;
    virtual std::map<QString, Program *> formatPrograms(EditingContext &context, const Buffer::Format &bufferFormat, const bool indexed, const Buffer::Format &paletteFormat) const override;
};

class ContourTool : public Tool {
public:
    using Tool::Tool;
    virtual std::map<QString, Program *> formatPrograms(EditingContext &context, const Buffer::Format &bufferFormat, const bool indexed, const Buffer::Format &paletteFormat) const override;
    virtual void end(EditingContext &context, const Mat4 &viewTransform) override;
    virtual void onCanvasPreview(EditingContext &context, const Mat4 &viewTransform, const bool isActive) override;
    virtual void onTopPreview(Editor &editor, EditingContext &context, const Mat4 &viewTransform, const bool isActive) override;
};

class ColourPickTool : public Tool {
public:
    enum class Mode {
        NodeColour,
        SceneColour,
    };

    explicit ColourPickTool() :
        Tool()
    {}
    virtual std::map<QString, Program *> formatPrograms(EditingContext &context, const Buffer::Format &bufferFormat, const bool indexed, const Buffer::Format &paletteFormat) const override;
    virtual void begin(EditingContext &context, const Mat4 &viewTransform) override;
    virtual void update(EditingContext &context, const Mat4 &viewTransform) override;
    virtual bool updatesContext() const override { return true; }
};

class TransformTool : public Tool {
public:
    explicit TransformTool(Editor &editor) :
        Tool(), editor(editor)
    {}
    virtual bool updatesViewTransform() const override { return true; }
    virtual bool isUndoable(EditingContext &context) const override;

protected:
    Editor &editor;
};

class TransformTargetOverrideTool : public TransformTool {
public:
    explicit TransformTargetOverrideTool(Editor &editor) :
        TransformTool(editor),
        oldTransformMode()
    {}
    virtual void begin(EditingContext &context, const Mat4 &viewTransform) override;
    virtual void end(EditingContext &context, const Mat4 &viewTransform) override;
    virtual bool isExclusive() const override { return false; }

protected:
    int oldTransformMode;
};

class PanTool : public TransformTool {
public:
    explicit PanTool(Editor &editor) :
        TransformTool(editor),
        oldViewportPos()
    {}
    virtual void begin(EditingContext &context, const Mat4 &viewTransform) override;
    virtual void update(EditingContext &context, const Mat4 &viewTransform) override;

protected:
    Vec2 oldViewportPos;
};

class RotoZoomTool : public TransformTool {
public:
    enum class Mode {
        RotoZoom,
        Zoom,
        Rotate,
    };
    explicit RotoZoomTool(Editor &editor) :
        TransformTool(editor),
        oldViewportPos()
    {}
    virtual void begin(EditingContext &context, const Mat4 &viewTransform) override;
    virtual void update(EditingContext &context, const Mat4 &viewTransform) override;
    virtual void onTopPreview(Editor &editor, EditingContext &context, const Mat4 &viewTransform, const bool isActive) override;

protected:
    Vec2 oldViewportPos;
};

class WheelZoomTool : public TransformTool {
public:
    explicit WheelZoomTool(Editor &editor) :
        TransformTool(editor)
    {}
    virtual void wheel(EditingContext &context, const Mat4 &viewTransform, const Vec2 &delta) override;
};

class WheelRotateTool : public TransformTool {
public:
    explicit WheelRotateTool(Editor &editor) :
        TransformTool(editor)
    {}
    virtual void wheel(EditingContext &context, const Mat4 &viewTransform, const Vec2 &delta) override;
};

} // namespace GfxPaint

#endif // TOOL_H
