#ifndef TOOL_H
#define TOOL_H

#include <QList>
#include <deque>

#include "types.h"
#include "stroke.h"

namespace GfxPaint {

class Editor;
class EditingContext;

class Tool {
public:
    Tool() {}
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

class StrokeTool : public Tool {
public:
    explicit StrokeTool() :
        Tool(),
        strokeOffset(0.0)
    {}
    virtual void begin(EditingContext &context, const Mat4 &viewTransform) override;
    virtual void update(EditingContext &context, const Mat4 &viewTransform) override;
    virtual void end(EditingContext &context, const Mat4 &viewTransform) override;
    virtual void onCanvasPreview(EditingContext &context, const Mat4 &viewTransform, const bool isActive) override;

protected:
    float strokeOffset;
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
};

class RectTool : public PrimitiveTool {
public:
    explicit RectTool() :
        PrimitiveTool()
    {}
    virtual void end(EditingContext &context, const Mat4 &viewTransform) override;
    virtual void onCanvasPreview(EditingContext &context, const Mat4 &viewTransform, const bool isActive) override;
    virtual void onTopPreview(Editor &editor, EditingContext &context, const Mat4 &viewTransform, const bool isActive) override;
};

class EllipseTool : public PrimitiveTool {
public:
    explicit EllipseTool() :
        PrimitiveTool()
    {}
    virtual void end(EditingContext &context, const Mat4 &viewTransform) override;
    virtual void onCanvasPreview(EditingContext &context, const Mat4 &viewTransform, const bool isActive) override;
    virtual void onTopPreview(Editor &editor, EditingContext &context, const Mat4 &viewTransform, const bool isActive) override;
};

class ContourTool : public Tool {
public:
    explicit ContourTool() :
        Tool()
    {}
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
    virtual void begin(EditingContext &context, const Mat4 &viewTransform) override;
    virtual void update(EditingContext &context, const Mat4 &viewTransform) override;
    virtual bool updatesContext() const override { return true; }

protected:
};

class TransformTool : public Tool {
public:
    explicit TransformTool(Editor &editor) :
        Tool(), editor(editor)
    {}
    virtual bool updatesContext() const override { return true; }
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
