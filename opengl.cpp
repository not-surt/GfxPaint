#include "opengl.h"

#include <QOffscreenSurface>
#include <QOpenGLContext>

#include "application.h"

namespace GfxPaint {

QSurfaceFormat OpenGL::format()
{
    QSurfaceFormat format;
    format.setVersion(OPENGL_MAJOR_VERSION, OPENGL_MINOR_VERSION);
    format.setProfile(QSurfaceFormat::CoreProfile);
    format.setOption(QSurfaceFormat::DebugContext);
    format.setRenderableType(QSurfaceFormat::OpenGL);
    return format;
}

void OpenGL::initialize()
{
    OpenGLFunctions::initializeOpenGLFunctions();
}

} // namespace GfxPaint
