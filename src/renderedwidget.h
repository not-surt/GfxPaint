#ifndef RENDEREDWIDGET_H
#define RENDEREDWIDGET_H

#include <QOpenGLWidget>
#include "opengl.h"

#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QBasicTimer>
#include <QElapsedTimer>

#include "buffer.h"
#include "rendermanager.h"

namespace GfxPaint {

class RenderedWidget : public QOpenGLWidget, protected OpenGL
{
public:
    explicit RenderedWidget(QWidget *const parent = nullptr);
    virtual ~RenderedWidget();

    static const Buffer::Format format;

protected:
    virtual void initializeGL() override;
    virtual void resizeGL(int w, int h) override;
    virtual void paintGL() override;

    void timerEvent(QTimerEvent *event) override;

    virtual void render() {}

    QMatrix4x4 mouseTransform;
    QMatrix4x4 viewportTransform;

    QOpenGLVertexArrayObject vao;

    Buffer *widgetBuffer;

    PatternProgram *patternProgram;
    RenderedWidgetProgram *widgetProgram;

    QBasicTimer repaintTimer;
    QElapsedTimer timer;
};

} // namespace GfxPaint

#endif // RENDEREDWIDGET_H
