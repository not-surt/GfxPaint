#include "coloursliderwidget.h"

#include "application.h"
#include "utils.h"

namespace GfxPaint {

const QVector<GLfloat> ColourSliderWidget::markerVertices = {
    0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
    -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
};

ColourSliderWidget::ColourSliderWidget(const ColourSpace colourSpace, const int component, const bool quantise, const Buffer::Format quantisePaletteFormat, QWidget *const parent) :
    RenderedWidget(parent),
    colourSpace(colourSpace), component(component), quantise(quantise), quantisePaletteFormat(quantisePaletteFormat),
    program(nullptr), pickProgram(nullptr), markerProgram(nullptr),
    m_colour(255, 0, 0, 255), m_pos(0.0),
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
    delete markerProgram;
}

void ColourSliderWidget::mouseEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        m_pos = clamp(0.0, 1.0, static_cast<qreal>(event->pos().x()) / static_cast<qreal>(width() - 1));
        emit posChanged(m_pos);
        setColour(pickProgram->pick(m_colour, m_pos, m_palette));
        event->accept();
    }
    else event->ignore();
}

void ColourSliderWidget::resizeGL(int w, int h)
{
    RenderedWidget::resizeGL(w, h);

    QList<Program *> oldPrograms = {program, pickProgram, markerProgram};
    ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
    program = new ColourSliderProgram(colourSpace, component, widgetBuffer->format(), 0, quantise, quantisePaletteFormat);
    //pickProgram = new ColourSliderPickProgram(colourSpace, component, quantise, m_palette ? m_palette->format() : Buffer::Format());
    pickProgram = new ColourSliderPickProgram(colourSpace, component, false, Buffer::Format());
    markerProgram = new GeometryProgram(widgetBuffer->format(), false, Buffer::Format(), 10, RenderManager::composeModeDefault);
    markerProgram->setGeometry(markerVertices);
    qDeleteAll(oldPrograms);
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

void ColourSliderWidget::setPos(const qreal pos)
{
    if (m_pos != pos) {
        m_pos = pos;
        update();
    }
}

void ColourSliderWidget::render()
{
    program->render(m_colour, colourSpace, component, RenderManager::unitToClipTransform, widgetBuffer, m_palette);
    QTransform markerTransform;
    markerTransform.translate(m_pos * 2.0 - 1.0, 0.0);
    markerTransform.scale((qreal)height() / (qreal)width(), 1.0);
    markerProgram->render(Qt::white, markerTransform, widgetBuffer, nullptr);
}

} // namespace GfxPaint
