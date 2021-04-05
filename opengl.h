#ifndef OPENGL_H
#define OPENGL_H

#include <QOpenGLExtraFunctions>
#include <QOpenGLWidget>
#include <QStringList>
#include <QDebug>
#include <QThread>

namespace GfxPaint {

using OpenGLFunctions = QOpenGLExtraFunctions;

class OpenGL : public OpenGLFunctions
{
public:
    explicit OpenGL(const bool autoInitialize = false) : OpenGLFunctions()
    {
        if (autoInitialize) {
            Q_ASSERT(QOpenGLContext::currentContext() != nullptr);
            initialize();
        }
    }
    virtual ~OpenGL() {}

    virtual void initialize();
};

class ContextBinder
{
public:
    explicit ContextBinder(QOpenGLContext *const context, QSurface *const surface)
        : previousContext(QOpenGLContext::currentContext()), previousSurface(previousContext ? previousContext->surface() : nullptr)
    {
        context->makeCurrent(surface);
        Q_ASSERT(surface && context);
        Q_ASSERT(QThread::currentThread() == context->thread());
        const bool makeCurrentSuccess = context->makeCurrent(surface);
        Q_ASSERT(makeCurrentSuccess);
    }
    explicit ContextBinder(QOpenGLWidget *const widget)
        : ContextBinder(widget->context(), widget->context()->surface()) {}
    ~ContextBinder()
    {
        QOpenGLContext::currentContext()->doneCurrent();
        if (previousContext) previousContext->makeCurrent(previousSurface);
    }

private:
    QOpenGLContext *const previousContext;
    QSurface *const previousSurface;
};

class FramebufferBinder : private OpenGL
{
public:
    explicit FramebufferBinder(const GLenum target = GL_FRAMEBUFFER, const GLuint framebuffer = 0, const QRect &viewport = QRect()) : OpenGL(true), target(target)
    {
        if (target == GL_FRAMEBUFFER || target == GL_READ_FRAMEBUFFER)
            glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &previousReadFramebuffer);
        if (target == GL_FRAMEBUFFER || target == GL_DRAW_FRAMEBUFFER)
            glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &previousDrawFramebuffer);
        if (framebuffer) glBindFramebuffer(target, framebuffer);
        glGetIntegerv(GL_VIEWPORT, previousViewport);
        glGetIntegerv(GL_SCISSOR_BOX, previousScissorBox);
        glGetBooleanv(GL_SCISSOR_TEST, &previousScissorTest);
        if (!viewport.isNull()) {
            glViewport(viewport.x(), viewport.y(), viewport.width(), viewport.height());
            glEnable(GL_SCISSOR_TEST);
            glScissor(viewport.x(), viewport.y(), viewport.width(), viewport.height());
        }
    }
    virtual ~FramebufferBinder() override
    {
        if (target == GL_FRAMEBUFFER || target == GL_READ_FRAMEBUFFER)
            glBindFramebuffer(GL_READ_FRAMEBUFFER, static_cast<GLuint>(previousReadFramebuffer));
        if (target == GL_FRAMEBUFFER || target == GL_DRAW_FRAMEBUFFER)
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, static_cast<GLuint>(previousDrawFramebuffer));
        glViewport(previousViewport[0], previousViewport[1], previousViewport[2], previousViewport[3]);
        if (previousScissorTest == GL_TRUE) glEnable(GL_SCISSOR_TEST);
        else glDisable(GL_SCISSOR_TEST);
        glScissor(previousScissorBox[0], previousScissorBox[1], previousScissorBox[2], previousScissorBox[3]);
    }

private:
    GLenum target;
    GLint previousReadFramebuffer;
    GLint previousDrawFramebuffer;
    GLint previousViewport[4];
    GLint previousScissorBox[4];
    GLboolean previousScissorTest;
};

class TextureBinder : private OpenGL
{
public:
    explicit TextureBinder(const GLenum target = GL_TEXTURE_2D, GLuint texture = 0) : OpenGL(true), target(target)
    {
        GLenum targetBinding;
        switch (target) {
        case GL_TEXTURE_2D: targetBinding = GL_TEXTURE_BINDING_2D; break;
        case GL_TEXTURE_RECTANGLE: targetBinding = GL_TEXTURE_BINDING_RECTANGLE; break;
        default:
            qDebug() << "unhandled texture target";
            targetBinding = GL_TEXTURE_BINDING_2D;
        }
        glGetIntegerv(targetBinding, &previousTexture);
        if (texture) glBindTexture(target, texture);
    }
    virtual ~TextureBinder() override
    {
        glBindTexture(target, static_cast<GLuint>(previousTexture));
    }
private:
    GLenum target;
    GLint previousTexture;
};

} // namespace GfxPaint

#endif // OPENGL_H
