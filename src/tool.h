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
    virtual void begin(const QVector2D viewportPos, const Point &point) {}
    virtual void update(const QVector2D viewportPos, const Point &point) {}
    virtual void end(const QVector2D viewportPos, const Point &point) {}
    virtual void preview(const QVector2D viewportPos, const Point &point) {}
    virtual void abort() {}
    virtual void wheel(const QVector2D pos, const QVector2D delta) {}

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
    virtual void begin(const QVector2D viewportPos, const Point &point) override;
    virtual void update(const QVector2D viewportPos, const Point &point) override;
    virtual void end(const QVector2D viewportPos, const Point &point) override;
    virtual void preview(const QVector2D viewportPos, const Point &point) override;
    virtual void abort() override {}

protected:
    float strokeOffset;
    Stroke strokePoints;
};

class PickTool : public Tool {
public:
    PickTool(Editor &editor) :
        Tool(editor)
    {}
    virtual void begin(const QVector2D viewportPos, const Point &point) override;
    virtual void update(const QVector2D viewportPos, const Point &point) override;
    virtual void end(const QVector2D viewportPos, const Point &point) override;
    virtual void abort() override {}

protected:
};

class PanTool : public Tool {
public:
    PanTool(Editor &editor) :
        Tool(editor),
        oldViewportPos()
    {}
    virtual void begin(const QVector2D viewportPos, const Point &point) override;
    virtual void update(const QVector2D viewportPos, const Point &point) override;
    virtual void end(const QVector2D viewportPos, const Point &point) override;
    virtual void abort() override {}

protected:
    QVector2D oldViewportPos;
};

class RotoZoomTool : public Tool {
public:
    RotoZoomTool(Editor &editor) :
        Tool(editor),
        oldViewportPos()
    {}
    virtual void begin(const QVector2D viewportPos, const Point &point) override;
    virtual void update(const QVector2D viewportPos, const Point &point) override;
    virtual void end(const QVector2D viewportPos, const Point &point) override;
    virtual void abort() override {}

protected:
    QVector2D oldViewportPos;
};

class ZoomTool : public Tool {
public:
    ZoomTool(Editor &editor) :
        Tool(editor)
    {}
    virtual void wheel(const QVector2D viewportPos, const QVector2D delta) override;
};

class RotateTool : public Tool {
public:
    RotateTool(Editor &editor) :
        Tool(editor)
    {}
    virtual void wheel(const QVector2D viewportPos, const QVector2D delta) override;
};

} // namespace GfxPaint

#endif // TOOL_H
