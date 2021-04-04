#include "opengl.h"

#include <QOffscreenSurface>
#include <QOpenGLContext>

#include "application.h"

namespace GfxPaint {

void OpenGL::initialize()
{
    OpenGLFunctions::initializeOpenGLFunctions();
}

} // namespace GfxPaint
