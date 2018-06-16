#ifndef TOOL_H
#define TOOL_H

#include <QDebug>
#include <QList>
#include <QPointF>

namespace GfxPaint {

class Editor;

class Tool {
public:
    Tool(Editor &editor) :
        editor(editor), active(false)
    {}
    virtual void begin(const QPointF point) {}
    virtual void update(const QPointF point) {}
    virtual void end(const QPointF point) {}
    virtual void abort() {}
    virtual void wheel(const QPointF point, const QPointF delta) {}

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
    virtual void begin(const QPointF point) override;
    virtual void update(const QPointF point) override;
    virtual void end(const QPointF point) override;
    virtual void abort() override {}

protected:
    qreal strokeOffset;
    QList<QPointF> strokePoints;
};

class PickTool : public Tool {
public:
    PickTool(Editor &editor) :
        Tool(editor)
    {}
    virtual void begin(const QPointF point) override;
    virtual void update(const QPointF point) override;
    virtual void end(const QPointF point) override;
    virtual void abort() override {}

protected:
};

class PanTool : public Tool {
public:
    PanTool(Editor &editor) :
        Tool(editor),
        oldPoint()
    {}
    virtual void begin(const QPointF point) override;
    virtual void update(const QPointF point) override;
    virtual void end(const QPointF point) override;
    virtual void abort() override {}

protected:
    QPointF oldPoint;
};

class RotoZoomTool : public Tool {
public:
    RotoZoomTool(Editor &editor) :
        Tool(editor),
        oldPoint()
    {}
    virtual void begin(const QPointF point) override;
    virtual void update(const QPointF point) override;
    virtual void end(const QPointF point) override;
    virtual void abort() override {}

protected:
    QPointF oldPoint;
};

class ZoomTool : public Tool {
public:
    ZoomTool(Editor &editor) :
        Tool(editor)
    {}
    virtual void wheel(const QPointF point, const QPointF delta) override;
};

class RotateTool : public Tool {
public:
    RotateTool(Editor &editor) :
        Tool(editor)
    {}
    virtual void wheel(const QPointF point, const QPointF delta) override;
};

} // namespace GfxPaint

#endif // TOOL_H
