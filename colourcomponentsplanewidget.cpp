#include "colourcomponentsplanewidget.h"

#include "application.h"
#include "utils.h"

namespace GfxPaint {

ColourComponentsPlaneWidget::ColourComponentsPlaneWidget(const ColourSpace colourSpace, const int xComponent, const int yComponent, const bool quantise, const Buffer::Format quantisePaletteFormat, QWidget *const parent) :
    RenderedWidget(parent),
    colourSpace(colourSpace), xComponent(xComponent), yComponent(yComponent), quantise(quantise), quantisePaletteFormat(quantisePaletteFormat),
    program(nullptr), pickProgram(nullptr), markerProgram(nullptr),
    m_pos(0.0, 0.0), m_colour{},
    m_palette(nullptr)
{
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);
}

ColourComponentsPlaneWidget::ColourComponentsPlaneWidget(QWidget *const parent) :
    ColourComponentsPlaneWidget(ColourSpace::HSL, 0, 1, false, Buffer::Format(), parent)
{
}

ColourComponentsPlaneWidget::~ColourComponentsPlaneWidget()
{
    ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
    delete program;
    delete pickProgram;
    delete markerProgram;
}

void ColourComponentsPlaneWidget::mouseEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        m_pos = QVector2D(clamp(0.0, 1.0, static_cast<qreal>(event->pos().x()) / static_cast<qreal>(width() - 1)),
                          clamp(0.0, 1.0, static_cast<qreal>(event->pos().y()) / static_cast<qreal>(height() - 1)));
        emit posChanged(m_pos);
        setColour(pickProgram->pick(m_colour, m_pos, m_palette));
        event->accept();
    }
    else event->ignore();
}

void ColourComponentsPlaneWidget::resizeGL(int w, int h)
{
    RenderedWidget::resizeGL(w, h);

    QList<Program *> oldPrograms = {program, pickProgram, markerProgram};
    ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
    program = new ColourPlaneProgram(colourSpace, true, true, widgetBuffer->format(), 0, quantise, quantisePaletteFormat);
    pickProgram = new ColourPlanePickProgram(colourSpace, xComponent, yComponent, quantise, m_palette ? m_palette->format() : Buffer::Format());
    markerProgram = new ModelProgram(widgetBuffer->format(), false, Buffer::Format(), 0, RenderManager::composeModeDefault);
    qDeleteAll(oldPrograms);
}

void ColourComponentsPlaneWidget::setColour(const Colour &colour)
{
    if (m_colour != colour) {
        m_colour = colour;
        emit colourChanged(colour);
        update();
    }
}

void ColourComponentsPlaneWidget::setPalette(const Buffer *const palette)
{
    if (m_palette != palette) {
        m_palette = palette;
        if (quantise) update();
    }
}

void ColourComponentsPlaneWidget::setPos(const QVector2D &pos)
{
    if (m_pos != pos) {
        m_pos = pos;
        emit posChanged(pos);
        update();
    }
}

void ColourComponentsPlaneWidget::render()
{
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
    program->render(m_colour, xComponent, yComponent, RenderManager::unitToClipTransform, widgetBuffer, m_palette);
    QMatrix4x4 markerTransform;
    markerTransform.translate(m_pos.x() * 2.0f - 1.0f, m_pos.y() * 2.0f - 1.0f);
    const float markerSize = std::min(sizeHint().width(), sizeHint().height()) / 4;
    markerTransform.scale(markerSize / (float)width(), markerSize / (float)height());
    markerProgram->render(qApp->renderManager.models["planeMarker"], {}, markerTransform, widgetBuffer, nullptr);
}

} // namespace GfxPaint
