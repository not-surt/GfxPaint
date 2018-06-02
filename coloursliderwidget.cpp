#include "coloursliderwidget.h"

#include "application.h"
#include "utils.h"

namespace GfxPaint {

ColourSliderWidget::ColourSliderWidget(const ColourSpace colourSpace, const int component, const bool quantise, const Buffer::Format quantisePaletteFormat, QWidget *const parent) :
    RenderedWidget(parent),
    colourSpace(colourSpace), component(component), quantise(quantise), quantisePaletteFormat(quantisePaletteFormat),
    program(nullptr), pickProgram(nullptr),
    m_colour(255, 0, 0, 255),
    m_palette(nullptr)
{

}

ColourSliderWidget::ColourSliderWidget(QWidget *const parent) :
    ColourSliderWidget(ColourSpace::HSL, 0, false, Buffer::Format(), parent)
{
}

ColourSliderWidget::~ColourSliderWidget()
{
    ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
    delete program;
    delete pickProgram;
}

bool ColourSliderWidget::mouseEvent(QMouseEvent *event)
{
    const float pos = clamp(0.0f, 1.0f, static_cast<float>(event->pos().x()) / static_cast<float>(width() - 1));
    setColour(pickProgram->pick(m_colour, pos, m_palette));
    event->accept();
    return true;
}

void ColourSliderWidget::resizeGL(int w, int h)
{
    RenderedWidget::resizeGL(w, h);

    ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
    Program *old = program;
    program = new ColourSliderProgram(colourSpace, component, widgetBuffer->format(), 0, quantise, quantisePaletteFormat);
    delete old;
    old = pickProgram;
    pickProgram = new ColourSliderPickProgram(colourSpace, component, quantise, m_palette ? m_palette->format() : Buffer::Format());
    delete old;
}

QColor ColourSliderWidget::colour() const
{
    return m_colour;
}

void ColourSliderWidget::setColour(const QColor &colour)
{
    if (m_colour != colour) {
        m_colour = colour;
        emit colourChanged(colour);
        update();
    }
}

void ColourSliderWidget::setPalette(const Buffer *const palette)
{
    if (m_palette != palette) {
        m_palette = palette;
        if (quantise) update();
    }
}

void ColourSliderWidget::render()
{
    program->render(m_colour, colourSpace, component, RenderManager::unitToClipTransform, widgetBuffer, m_palette);
}

} // namespace GfxPaint
