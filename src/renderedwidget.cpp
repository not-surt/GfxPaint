#include "renderedwidget.h"

#include <cmath>

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
    const float framesPerSecond = 60.0f;
    repaintTimer.start(1000.0f / framesPerSecond, this);
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

    {
        ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);

        std::list<Program *> oldPrograms = {widgetProgram, patternProgram};
        widgetProgram = new RenderedWidgetProgram(format, false, Buffer::Format());
        patternProgram = new BackgroundCheckersProgram(Pattern::Checkers, format, 0);
        oldPrograms.clear();
    }
}

void RenderedWidget::resizeGL(int w, int h)
{
    QOpenGLWidget::resizeGL(w, h);

    const float data[] = {
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        std::floor(w / 2.0f), std::floor(h / 2.0f), 0.0f, 1.0f
    };
    Mat4 originTransform;
    memcpy(originTransform.data(), data, sizeof(data));
    mouseTransform = originTransform.inverted();
    viewportTransform = GfxPaint::viewportToClipTransform({w, h}) * originTransform;

    {
        ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
        delete widgetBuffer;
        widgetBuffer = new Buffer(QSize(w, h), format);
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
//    const float time = timer.elapsed() / 1000.0f;
    const float time = qApp->time();
    const float degreesPerSecond = 45.0f;
    Mat4 transform;
    transform.scale(16.0f, 16.0f);
    transform.rotate(time * degreesPerSecond, {0.0f, 0.0f, 1.0f});
    transform.translate(0.0f, 1.0f);
    transform.rotate(-time * degreesPerSecond, {0.0f, 0.0f, 1.0f});
    transform = viewportTransform * transform;
    patternProgram->render(RenderManager::flipTransform * transform);

    // Draw buffer
    glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_BLEND);
    Mat4 matrix;
    matrix.scale(width(), height());
    widgetProgram->render(widgetBuffer, viewportTransform);
}

void RenderedWidget::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == repaintTimer.timerId()) update();
    else QWidget::timerEvent(event);
}

} // namespace GfxPaint
