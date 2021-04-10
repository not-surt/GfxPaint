#include "colourpalettewidget.h"

#include "application.h"

namespace GfxPaint {

ColourPaletteWidget::ColourPaletteWidget(QWidget *const parent) :
    RenderedWidget(parent),
    swatchSize(16, 16), columns(16),
    program(nullptr),
    m_palette(nullptr)
{
}

ColourPaletteWidget::~ColourPaletteWidget()
{
    ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
    delete program;
}

void ColourPaletteWidget::setPalette(const Buffer *const palette)
{
    m_palette = palette;
    resizeGL(width(), height());
}

void ColourPaletteWidget::mouseEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
//        m_pos = clamp(0.0, 1.0, static_cast<qreal>(event->pos().x()) / static_cast<qreal>(width() - 1));
//        emit posChanged(m_pos);
//        setColour(pickProgram->pick(m_colour, m_pos, m_palette));
        event->accept();
    }
    else event->ignore();
}

void ColourPaletteWidget::resizeGL(int w, int h)
{
    RenderedWidget::resizeGL(w, h);

    QList<Program *> oldPrograms = {program};
    ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
    program = new ColourPaletteProgram(widgetBuffer->format(), 0, m_palette ? m_palette->format() : Buffer::Format());
    qDeleteAll(oldPrograms);
}

void ColourPaletteWidget::render()
{
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    program->render(m_palette, size(), swatchSize, QSize(16, 16), RenderManager::unitToClipTransform, widgetBuffer);
}

} // namespace GfxPaint
