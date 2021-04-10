#include "colourcomponentsliderwidget.h"

#include "application.h"
#include "utils.h"

namespace GfxPaint {

ColourComponentSliderWidget::ColourComponentSliderWidget(const ColourSpace colourSpace, const int component, const bool quantise, const Buffer::Format quantisePaletteFormat, QWidget *const parent) :
    RenderedWidget(parent),
    colourSpace(colourSpace), component(component), quantise(quantise), quantisePaletteFormat(quantisePaletteFormat),
    program(nullptr), markerProgram(nullptr),
    m_pos(0.0), m_colour{},
    m_palette(nullptr)
{
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed);
}

ColourComponentSliderWidget::ColourComponentSliderWidget(QWidget *const parent) :
    ColourComponentSliderWidget(ColourSpace::HSL, 0, false, Buffer::Format(), parent)
{
}

ColourComponentSliderWidget::~ColourComponentSliderWidget()
{
    ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
    delete program;
    delete markerProgram;
}

void ColourComponentSliderWidget::mouseEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        setPos(clamp(0.0, 1.0, static_cast<qreal>(event->pos().x()) / static_cast<qreal>(width() - 1)));
        event->accept();
    }
    else event->ignore();
}

void ColourComponentSliderWidget::resizeGL(int w, int h)
{
    RenderedWidget::resizeGL(w, h);

    QList<Program *> oldPrograms = {program, markerProgram};
    ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
    program = new ColourPlaneProgram(colourSpace, true, false, widgetBuffer->format(), 0, quantise, quantisePaletteFormat);
    markerProgram = new ModelProgram(widgetBuffer->format(), false, Buffer::Format(), 0, RenderManager::composeModeDefault);
    qDeleteAll(oldPrograms);
}

void ColourComponentSliderWidget::setColour(const Colour &colour)
{
    if (m_colour != colour) {
        m_colour = colour;
        update();
    }
}

void ColourComponentSliderWidget::setPalette(const Buffer *const palette)
{
    if (m_palette != palette) {
        m_palette = palette;
        if (quantise) update();
    }
}

void ColourComponentSliderWidget::setPos(const qreal pos)
{
    if (m_pos != pos) {
        m_pos = pos;
        emit posChanged(pos);
        update();
    }
}

void ColourComponentSliderWidget::render()
{
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    program->render(m_colour, component, -1, RenderManager::unitToClipTransform, widgetBuffer, m_palette);
    QMatrix4x4 markerTransform;
    markerTransform.translate(m_pos * 2.0f - 1.0f, 0.0f);
    const float markerSize = sizeHint().width() / 4.0;
    markerTransform.scale(markerSize / (float)width(), 1.0);
    markerProgram->render(qApp->renderManager.models["sliderMarker"], {}, markerTransform, widgetBuffer, nullptr);
}

} // namespace GfxPaint
