#ifndef TOOL_H
#define TOOL_H

#include <QList>
#include <deque>

#include "types.h"
#include "stroke.h"

namespace GfxPaint {

class Editor;

class Tool {
public:
    explicit Tool(Editor &editor) :
        editor(editor), active(false)
    {}
    virtual void begin(const Vec2 &viewportPos, const Point &point, const int mode) {}
    virtual void update(const Vec2 &viewportPos, const Point &point, const int mode) {}
    virtual void end(const Vec2 &viewportPos, const Point &point, const int mode) {}
    virtual void onCanvasPreview(const Vec2 &viewportPos, const Point &point, const bool isActive, const int mode) {}
    virtual void onTopPreview(const Vec2 &viewportPos, const Point &point, const bool isActive, const int mode) {}
    virtual void abort(const int mode) {}
    virtual void wheel(const Vec2 &viewportPos, const Vec2 &delta, const int mode) {}
    virtual bool isExclusive() const { return true; }

protected:
    Editor &editor;
    bool active;
};

class StrokeTool : public Tool {
public:
    explicit StrokeTool(Editor &editor) :
        Tool(editor),
        strokeOffset(0.0), strokePoints()
    {}
    virtual void begin(const Vec2 &viewportPos, const Point &point, const int mode) override;
    virtual void update(const Vec2 &viewportPos, const Point &point, const int mode) override;
    virtual void end(const Vec2 &viewportPos, const Point &point, const int mode) override;
    virtual void onCanvasPreview(const Vec2 &viewportPos, const Point &point, const bool isActive, const int mode) override;

protected:
    float strokeOffset;
    Stroke strokePoints;
};

class RectTool : public Tool {
public:
    explicit RectTool(Editor &editor) :
        Tool(editor),
        worldSpacePoints()
    {}
    virtual void begin(const Vec2 &viewportPos, const Point &point, const int mode) override;
    virtual void update(const Vec2 &viewportPos, const Point &point, const int mode) override;
    virtual void end(const Vec2 &viewportPos, const Point &point, const int mode) override;
    virtual void onCanvasPreview(const Vec2 &viewportPos, const Point &point, const bool isActive, const int mode) override;
    virtual void onTopPreview(const Vec2 &viewportPos, const Point &point, const bool isActive, const int mode) override;

protected:
    std::array<Vec2, 2> worldSpacePoints;
};

class EllipseTool : public Tool {
public:
    explicit EllipseTool(Editor &editor) :
        Tool(editor),
        points()
    {}
    virtual void begin(const Vec2 &viewportPos, const Point &point, const int mode) override;
    virtual void update(const Vec2 &viewportPos, const Point &point, const int mode) override;
    virtual void end(const Vec2 &viewportPos, const Point &point, const int mode) override;
    virtual void onCanvasPreview(const Vec2 &viewportPos, const Point &point, const bool isActive, const int mode) override;
    virtual void onTopPreview(const Vec2 &viewportPos, const Point &point, const bool isActive, const int mode) override;

protected:
    std::array<Vec2, 2> points;
};

class ContourTool : public Tool {
public:
    explicit ContourTool(Editor &editor) :
        Tool(editor),
        points()
    {}
    virtual void begin(const Vec2 &viewportPos, const Point &point, const int mode) override;
    virtual void update(const Vec2 &viewportPos, const Point &point, const int mode) override;
    virtual void end(const Vec2 &viewportPos, const Point &point, const int mode) override;
    virtual void onCanvasPreview(const Vec2 &viewportPos, const Point &point, const bool isActive, const int mode) override;
    virtual void onTopPreview(const Vec2 &viewportPos, const Point &point, const bool isActive, const int mode) override;

protected:
    std::vector<Vec2> points;
};

class ColourPickTool : public Tool {
public:
    explicit ColourPickTool(Editor &editor) :
        Tool(editor)
    {}
    virtual void begin(const Vec2 &viewportPos, const Point &point, const int mode) override;
    virtual void update(const Vec2 &viewportPos, const Point &point, const int mode) override;

protected:
};

class TransformTargetOverrideTool : public Tool {
public:
    explicit TransformTargetOverrideTool(Editor &editor) :
        Tool(editor),
        oldTransformMode()
    {}
    virtual void begin(const Vec2 &viewportPos, const Point &point, const int mode) override;
    virtual void end(const Vec2 &viewportPos, const Point &point, const int mode) override;
    virtual bool isExclusive() const override { return false; }

protected:
    int oldTransformMode;
};

class PanTool : public Tool {
public:
    explicit PanTool(Editor &editor) :
        Tool(editor),
        oldViewportPos()
    {}
    virtual void begin(const Vec2 &viewportPos, const Point &point, const int mode) override;
    virtual void update(const Vec2 &viewportPos, const Point &point, const int mode) override;

protected:
    Vec2 oldViewportPos;
};

class RotoZoomTool : public Tool {
public:
    enum class Mode {
        RotoZoom,
        Zoom,
        Rotate,
    };
    explicit RotoZoomTool(Editor &editor) :
        Tool(editor),
        oldViewportPos()
    {}
    virtual void begin(const Vec2 &viewportPos, const Point &point, const int mode) override;
    virtual void update(const Vec2 &viewportPos, const Point &point, const int mode) override;
    virtual void onTopPreview(const Vec2 &viewportPos, const Point &point, const bool isActive, const int mode) override;

protected:
    Vec2 oldViewportPos;
};

class WheelZoomTool : public Tool {
public:
    explicit WheelZoomTool(Editor &editor) :
        Tool(editor)
    {}
    virtual void wheel(const Vec2 &viewportPos, const Vec2 &delta, const int mode) override;
};

class WheelRotateTool : public Tool {
public:
    explicit WheelRotateTool(Editor &editor) :
        Tool(editor)
    {}
    virtual void wheel(const Vec2 &viewportPos, const Vec2 &delta, const int mode) override;
};

} // namespace GfxPaint

#endif // TOOL_H
