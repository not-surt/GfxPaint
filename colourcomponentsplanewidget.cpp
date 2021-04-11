#include "colourcomponentsplanewidget.h"

#include "application.h"
#include "utils.h"

namespace GfxPaint {

ColourComponentsPlaneWidget::ColourComponentsPlaneWidget(const ColourSpace colourSpace, const int xComponent, const int yComponent, const bool quantise, const Buffer::Format quantisePaletteFormat, QWidget *const parent) :
    RenderedWidget(parent),
    colourSpace(colourSpace), xComponent(xComponent), yComponent(yComponent), quantise(quantise), quantisePaletteFormat(quantisePaletteFormat),
    program(nullptr), markerProgram(nullptr),
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
    delete markerProgram;
}

void ColourComponentsPlaneWidget::mouseEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton) {
        m_pos = QVector2D(clamp(0.0, 1.0, static_cast<qreal>(event->pos().x()) / static_cast<qreal>(width() - 1)),
                          clamp(0.0, 1.0, static_cast<qreal>(event->pos().y()) / static_cast<qreal>(height() - 1)));
        emit posChanged(m_pos);
        event->accept();
    }
    else event->ignore();
}

void ColourComponentsPlaneWidget::setColour(const Colour &colour)
{
    if (m_colour != colour) {
        m_colour = colour;
        update();
    }
}

void ColourComponentsPlaneWidget::setPalette(const Buffer *const palette)
{
    QList<Program *> oldPrograms = {program};
    ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
    program = new ColourPlaneProgram(colourSpace, true, true, RenderedWidget::format, 0, quantise, quantisePaletteFormat);
    qDeleteAll(oldPrograms);
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

void ColourComponentsPlaneWidget::initializeGL()
{
    RenderedWidget::initializeGL();

    {
        ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
        QList<Program *> oldPrograms = {markerProgram};
        markerProgram = new ModelProgram(RenderedWidget::format, false, Buffer::Format(), 0, RenderManager::composeModeDefault);
        qDeleteAll(oldPrograms);
    }
    setPalette(m_palette);
}

void ColourComponentsPlaneWidget::render()
{
    program->render(m_colour, xComponent, yComponent, RenderManager::unitToClipTransform, widgetBuffer, m_palette);
    QMatrix4x4 markerTransform;
    markerTransform.translate(m_pos.x() * 2.0f - 1.0f, m_pos.y() * 2.0f - 1.0f);
    const float markerSize = std::min(sizeHint().width(), sizeHint().height()) / 4;
    markerTransform.scale(markerSize / (float)width(), markerSize / (float)height());
    markerProgram->render(qApp->renderManager.models["planeMarker"], {}, markerTransform, widgetBuffer, nullptr);
}

} // namespace GfxPaint
