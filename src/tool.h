#ifndef TOOL_H
#define TOOL_H

#include <QList>
#include <QVector2D>

#include "stroke.h"

namespace GfxPaint {

class Editor;

class Tool {
public:
    Tool(Editor &editor) :
        editor(editor), active(false)
    {}
    virtual void begin(const QVector2D &viewportPos, const Point &point, const int mode = 0) {}
    virtual void update(const QVector2D &viewportPos, const Point &point, const int mode = 0) {}
    virtual void end(const QVector2D &viewportPos, const Point &point, const int mode = 0) {}
    virtual void onCanvasPreview(const QVector2D &viewportPos, const Point &point, const int mode = 0) {}
    virtual void onTopPreview(const QVector2D &viewportPos, const Point &point, const int mode = 0) {}
    virtual void abort(const int mode = 0) {}
    virtual void wheel(const QVector2D &viewportPos, const QVector2D &delta, const int mode = 0) {}

protected:
    Editor &editor;
    bool active;
};

class StrokeTool : public Tool {
public:
    StrokeTool(Editor &editor) :
        Tool(editor),
        strokeOffset(0.0), strokePoints()
    {}
    virtual void begin(const QVector2D &viewportPos, const Point &point, const int mode = 0) override;
    virtual void update(const QVector2D &viewportPos, const Point &point, const int mode = 0) override;
    virtual void end(const QVector2D &viewportPos, const Point &point, const int mode = 0) override;
    virtual void onCanvasPreview(const QVector2D &viewportPos, const Point &point, const int mode = 0) override;
    virtual void abort(const int mode = 0) override {}

protected:
    float strokeOffset;
    Stroke strokePoints;
};

class RectTool : public Tool {
public:
    RectTool(Editor &editor) :
        Tool(editor),
        points()
    {}
    virtual void begin(const QVector2D &viewportPos, const Point &point, const int mode = 0) override;
    virtual void update(const QVector2D &viewportPos, const Point &point, const int mode = 0) override;
    virtual void end(const QVector2D &viewportPos, const Point &point, const int mode = 0) override;
    virtual void onCanvasPreview(const QVector2D &viewportPos, const Point &point, const int mode = 0) override;
    virtual void abort(const int mode = 0) override {}

protected:
    float strokeOffset;
    std::array<QVector2D, 2> points;
};

class PickTool : public Tool {
public:
    PickTool(Editor &editor) :
        Tool(editor)
    {}
    virtual void begin(const QVector2D &viewportPos, const Point &point, const int mode = 0) override;
    virtual void update(const QVector2D &viewportPos, const Point &point, const int mode = 0) override;
    virtual void end(const QVector2D &viewportPos, const Point &point, const int mode = 0) override;
    virtual void abort(const int mode = 0) override {}

protected:
};

class PanTool : public Tool {
public:
    PanTool(Editor &editor) :
        Tool(editor),
        oldViewportPos()
    {}
    virtual void begin(const QVector2D &viewportPos, const Point &point, const int mode = 0) override;
    virtual void update(const QVector2D &viewportPos, const Point &point, const int mode = 0) override;
    virtual void end(const QVector2D &viewportPos, const Point &point, const int mode = 0) override;
    virtual void abort(const int mode = 0) override {}

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
    RotoZoomTool(Editor &editor) :
        Tool(editor),
        oldViewportPos()
    {}
    virtual void begin(const QVector2D &viewportPos, const Point &point, const int mode = 0) override;
    virtual void update(const QVector2D &viewportPos, const Point &point, const int mode = 0) override;
    virtual void end(const QVector2D &viewportPos, const Point &point, const int mode = 0) override;
    virtual void onTopPreview(const QVector2D &viewportPos, const Point &point, const int mode = 0) override;
    virtual void abort(const int mode = 0) override {}

protected:
    QVector2D oldViewportPos;
};

class ZoomTool : public Tool {
public:
    ZoomTool(Editor &editor) :
        Tool(editor)
    {}
    virtual void wheel(const QVector2D &viewportPos, const QVector2D &delta, const int mode = 0) override;
};

class RotateTool : public Tool {
public:
    RotateTool(Editor &editor) :
        Tool(editor)
    {}
    virtual void wheel(const QVector2D &viewportPos, const QVector2D &delta, const int mode = 0) override;
};

} // namespace GfxPaint

#endif // TOOL_H
