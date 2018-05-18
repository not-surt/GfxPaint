#include "renderedwidget.h"

#include "application.h"
#include "utils.h"

namespace GfxPaint {

const Buffer::Format RenderedWidget::format = Buffer::Format(BufferData::Format::ComponentType::UInt, 1, 4);

RenderedWidget::RenderedWidget(QWidget *const parent) :
    QOpenGLWidget(parent), OpenGL(),
    mouseTransform(), viewportTransform(),
    vao(),
    widgetBuffer(nullptr),
    patternProgram(nullptr), widgetProgram(nullptr),
    repaintTimer(), timer()
{
    const float framesPerSecond = 60.0;
    repaintTimer.start(1000 / framesPerSecond, this);
    timer.start();
}

RenderedWidget::~RenderedWidget()
{
    repaintTimer.stop();
    if (context()) {
        ContextBinder contextBinder(this);
        vao.destroy();
    }
    {
        ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
        delete widgetProgram;
        delete patternProgram;
        delete widgetBuffer;
    }
}

void RenderedWidget::initializeGL()
{
    QOpenGLWidget::initializeGL();

    initializeOpenGLFunctions();

    vao.create();
    vao.bind();
}

void RenderedWidget::resizeGL(int w, int h)
{
    QOpenGLWidget::resizeGL(w, h);

    const QTransform originTransform(
        1.0, 0.0, 0.0,
        0.0, 1.0, 0.0,
        floor(w / 2.0), floor(h / 2.0), 1.0
    );
    mouseTransform = originTransform.inverted();
    viewportTransform = originTransform * GfxPaint::viewportTransform({w, h});

    {
        ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
        delete widgetBuffer;
        widgetBuffer = new Buffer(QSize(w, h), format);

        Program *old;

        old = widgetProgram;
        widgetProgram = new WidgetProgram(widgetBuffer->format(), false, Buffer::Format());
        delete old;

        old = patternProgram;
        patternProgram = new PatternProgram(Pattern::Checkers, widgetBuffer->format(), Blender::Alpha);
        delete old;
    }
}

void RenderedWidget::paintGL()
{
    QOpenGLWidget::paintGL();

    // Render to buffer
    {
        ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
        widgetBuffer->clear();
        widgetBuffer->bindFramebuffer();

        glDisable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);

        render();
    }

    // Draw checkers
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    const qreal time = timer.elapsed() / 1000.0;
    const qreal degreesPerSecond = 45.0;
    QTransform backgroundTransform;
    backgroundTransform.scale(16.0, 16.0);
    backgroundTransform.rotate(time * degreesPerSecond);
    backgroundTransform.translate(0.0, 1.0);
    backgroundTransform.rotate(-time * degreesPerSecond);
    //backgroundTransform.translate(time, time);
    patternProgram->render(backgroundTransform * viewportTransform * RenderManager::flipTransform);

    // Draw buffer
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    widgetProgram->render(widgetBuffer);
}

void RenderedWidget::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == repaintTimer.timerId()) update();
    else QWidget::timerEvent(event);
}

} // namespace GfxPaint
