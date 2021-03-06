#include "coloursliderwidget.h"

#include "application.h"
#include "utils.h"

namespace GfxPaint {

const QVector<GLfloat> ColourSliderWidget::markerVertices = {
    0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f,
    0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
    -1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f,
    0.0f, 0.0f, 1.0f, 1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
    0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 1.0f,
};

const QVector<GLushort> ColourSliderWidget::markerIndices = {
    0, 1, 2,
    3, 4, 5,
};

const QVector<GLushort> ColourSliderWidget::markerElements = {
    3,
    3,
};

ColourSliderWidget::ColourSliderWidget(const ColourSpace colourSpace, const int component, const bool quantise, const Buffer::Format quantisePaletteFormat, QWidget *const parent) :
    RenderedWidget(parent),
    colourSpace(colourSpace), component(component), quantise(quantise), quantisePaletteFormat(quantisePaletteFormat),
    program(nullptr), pickProgram(nullptr), markerProgram(nullptr),
    m_pos(0.0), m_colour(255, 0, 0, 255),
    m_palette(nullptr), markerModel(nullptr)
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
    delete markerModel;
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
    markerProgram = new ModelProgram(widgetBuffer->format(), false, Buffer::Format(), 0, RenderManager::composeModeDefault);
    qDeleteAll(oldPrograms);
    delete markerModel;
    markerModel = new Model(GL_TRIANGLES, markerVertices, markerIndices, markerElements);
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
        emit posChanged(pos);
        update();
    }
}

void ColourSliderWidget::render()
{
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    program->render(m_colour, colourSpace, component, RenderManager::unitToClipTransform, widgetBuffer, m_palette);
    QTransform markerTransform;
    markerTransform.translate(m_pos * 2.0 - 1.0, 0.0);
    markerTransform.scale((qreal)height() / (qreal)width(), 1.0);
    markerProgram->render(markerModel, Qt::white, markerTransform, widgetBuffer, nullptr);
}

} // namespace GfxPaint
