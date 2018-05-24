#include "coloursliderwidget.h"

#include "application.h"
#include "utils.h"

namespace GfxPaint {

ColourSliderWidget::ColourSliderWidget(const ColourSpace colourSpace, const int component, QWidget *const parent) :
    RenderedWidget(parent),
    colourSpace(colourSpace), component(component),
    program(nullptr),
    m_colour(255, 0, 0, 255)
{

}

ColourSliderWidget::ColourSliderWidget(QWidget *const parent) :
    ColourSliderWidget(ColourSpace::HSL, 0, parent)
{
}

ColourSliderWidget::~ColourSliderWidget()
{
    ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
    delete program;
}

bool ColourSliderWidget::mouseEvent(QMouseEvent *event)
{
    const qreal pos = clamp(0.0, 1.0, static_cast<qreal>(event->pos().x()) / static_cast<qreal>(width() - 1));
    QColor colour = m_colour;
    colour.setRedF(pos);
    setColour(colour);
    event->accept();
    return true;
}

void ColourSliderWidget::resizeGL(int w, int h)
{
    RenderedWidget::resizeGL(w, h);

    ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
    Program *const old = program;
    program = new ColourSliderProgram(colourSpace, component, widgetBuffer->format(), Blender::Alpha);
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

void ColourSliderWidget::render()
{
    program->render(m_colour, colourSpace, component, RenderManager::unitToClipTransform, widgetBuffer);
}

} // namespace GfxPaint
