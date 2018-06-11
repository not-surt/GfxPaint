#ifndef TOOL_H
#define TOOL_H

#include <QList>
#include <QPointF>

namespace GfxPaint {

class Editor;

class Tool {
public:
    Tool(Editor &editor) :
        editor(editor)
    {}
    virtual void begin(const QPointF point) = 0;
    virtual void update(const QPointF point) = 0;
    virtual void end(const QPointF point) = 0;
    virtual void abort() = 0;

protected:
    Editor &editor;
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

class RotoScaleTool : public Tool {
public:
    RotoScaleTool(Editor &editor) :
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

} // namespace GfxPaint

#endif // TOOL_H
