#ifndef TOOL_H
#define TOOL_H

#include <QList>
#include <QVector2D>
#include <deque>

#include "stroke.h"

namespace GfxPaint {

class Editor;

class Tool {
public:
    explicit Tool(Editor &editor) :
        editor(editor), active(false)
    {}
    virtual void begin(const QVector2D &viewportPos, const Point &point, const int mode = 0) {}
    virtual void update(const QVector2D &viewportPos, const Point &point, const int mode = 0) {}
    virtual void end(const QVector2D &viewportPos, const Point &point, const int mode = 0) {}
    virtual void onCanvasPreview(const QVector2D &viewportPos, const Point &point, const int mode = 0) {}
    virtual void onTopPreview(const QVector2D &viewportPos, const Point &point, const int mode = 0) {}
    virtual void abort(const int mode = 0) {}
    virtual void wheel(const QVector2D &viewportPos, const QVector2D &delta, const int mode = 0) {}
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
    virtual void begin(const QVector2D &viewportPos, const Point &point, const int mode = 0) override;
    virtual void update(const QVector2D &viewportPos, const Point &point, const int mode = 0) override;
    virtual void end(const QVector2D &viewportPos, const Point &point, const int mode = 0) override;
    virtual void onCanvasPreview(const QVector2D &viewportPos, const Point &point, const int mode = 0) override;

protected:
    float strokeOffset;
    Stroke strokePoints;
};

class RectTool : public Tool {
public:
    explicit RectTool(Editor &editor) :
        Tool(editor),
        points()
    {}
    virtual void begin(const QVector2D &viewportPos, const Point &point, const int mode = 0) override;
    virtual void update(const QVector2D &viewportPos, const Point &point, const int mode = 0) override;
    virtual void end(const QVector2D &viewportPos, const Point &point, const int mode = 0) override;
    virtual void onCanvasPreview(const QVector2D &viewportPos, const Point &point, const int mode = 0) override;
    virtual void onTopPreview(const QVector2D &viewportPos, const Point &point, const int mode = 0) override;

protected:
    std::array<QVector2D, 2> points;
};

class EllipseTool : public Tool {
public:
    explicit EllipseTool(Editor &editor) :
        Tool(editor),
        points()
    {}
    virtual void begin(const QVector2D &viewportPos, const Point &point, const int mode = 0) override;
    virtual void update(const QVector2D &viewportPos, const Point &point, const int mode = 0) override;
    virtual void end(const QVector2D &viewportPos, const Point &point, const int mode = 0) override;
    virtual void onCanvasPreview(const QVector2D &viewportPos, const Point &point, const int mode = 0) override;
    virtual void onTopPreview(const QVector2D &viewportPos, const Point &point, const int mode = 0) override;

protected:
    std::array<QVector2D, 2> points;
};

class ContourTool : public Tool {
public:
    explicit ContourTool(Editor &editor) :
        Tool(editor),
        points()
    {}
    virtual void begin(const QVector2D &viewportPos, const Point &point, const int mode = 0) override;
    virtual void update(const QVector2D &viewportPos, const Point &point, const int mode = 0) override;
    virtual void end(const QVector2D &viewportPos, const Point &point, const int mode = 0) override;
    virtual void onCanvasPreview(const QVector2D &viewportPos, const Point &point, const int mode = 0) override;
    virtual void onTopPreview(const QVector2D &viewportPos, const Point &point, const int mode = 0) override;

protected:
    std::vector<QVector2D> points;
};

class PickTool : public Tool {
public:
    explicit PickTool(Editor &editor) :
        Tool(editor)
    {}
    virtual void begin(const QVector2D &viewportPos, const Point &point, const int mode = 0) override;
    virtual void update(const QVector2D &viewportPos, const Point &point, const int mode = 0) override;

protected:
};

class TransformTargetOverrideTool : public Tool {
public:
    explicit TransformTargetOverrideTool(Editor &editor) :
        Tool(editor),
        oldTransformMode()
    {}
    virtual void begin(const QVector2D &viewportPos, const Point &point, const int mode = 0) override;
    virtual void end(const QVector2D &viewportPos, const Point &point, const int mode = 0) override;
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
    virtual void begin(const QVector2D &viewportPos, const Point &point, const int mode = 0) override;
    virtual void update(const QVector2D &viewportPos, const Point &point, const int mode = 0) override;

protected:
    QVector2D oldViewportPos;
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
    virtual void begin(const QVector2D &viewportPos, const Point &point, const int mode = 0) override;
    virtual void update(const QVector2D &viewportPos, const Point &point, const int mode = 0) override;
    virtual void onTopPreview(const QVector2D &viewportPos, const Point &point, const int mode = 0) override;

protected:
    QVector2D oldViewportPos;
};

class ZoomTool : public Tool {
public:
    explicit ZoomTool(Editor &editor) :
        Tool(editor)
    {}
    virtual void wheel(const QVector2D &viewportPos, const QVector2D &delta, const int mode = 0) override;
};

class RotateTool : public Tool {
public:
    explicit RotateTool(Editor &editor) :
        Tool(editor)
    {}
    virtual void wheel(const QVector2D &viewportPos, const QVector2D &delta, const int mode = 0) override;
};

} // namespace GfxPaint

#endif // TOOL_H
