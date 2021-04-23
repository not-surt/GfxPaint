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
    setSizePolicy(xComponent >= 0 ? QSizePolicy::MinimumExpanding : QSizePolicy::Fixed,
                  yComponent >= 0 ? QSizePolicy::MinimumExpanding : QSizePolicy::Fixed);
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
        m_pos = Vec2(xComponent >= 0 ? clamp(0.0f, 1.0f, static_cast<float>(event->pos().x()) / static_cast<float>(width() - 1)) : 0.5f,
                          yComponent >= 0 ? clamp(0.0f, 1.0f, static_cast<float>(event->pos().y()) / static_cast<float>(height() - 1)) : 0.5f);
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
    program = new ColourPlaneProgram(colourSpace, xComponent >= 0, yComponent >= 0, RenderedWidget::format, 0, quantise, quantisePaletteFormat);
    qDeleteAll(oldPrograms);
    if (m_palette != palette) {
        m_palette = palette;
        if (quantise) update();
    }
}

void ColourComponentsPlaneWidget::setPos(const Vec2 &pos)
{
    if (m_pos != pos) {
        m_pos = Vec2(xComponent >= 0 ? pos.x() : 0.5f, yComponent >= 0 ? pos.y() : 0.5f);
        emit posChanged(m_pos);
        update();
    }
}

void ColourComponentsPlaneWidget::initializeGL()
{
    RenderedWidget::initializeGL();

    {
        ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
        QList<Program *> oldPrograms = {markerProgram};
        markerProgram = new VertexColourModelProgram(RenderedWidget::format, false, Buffer::Format(), 0, RenderManager::composeModeDefault);
        qDeleteAll(oldPrograms);
    }
    setPalette(m_palette);
}

void ColourComponentsPlaneWidget::render()
{
    program->render(m_colour, xComponent, yComponent, RenderManager::unitToClipTransform, widgetBuffer, m_palette);
    Mat4 markerTransform;
    markerTransform.translate(m_pos.x() * 2.0f - 1.0f, m_pos.y() * 2.0f - 1.0f);
    float markerSize = std::min((float)sizeHint().width(), (float)sizeHint().height());
    if (xComponent >= 0 && yComponent >= 0) markerSize /= 4;
    markerTransform.scale(markerSize / (float)width(), markerSize / (float)height());
    Model *markerModel = nullptr;
    if (xComponent >= 0 && yComponent >= 0) {
        markerModel = qApp->renderManager.models["planeMarker"];
    }
    else {
        markerModel = qApp->renderManager.models["sliderMarker"];
        if (yComponent >= 0) {
            markerTransform.rotate(90.0f);
        }
    }
    markerProgram->render(markerModel, markerTransform, widgetBuffer, nullptr);
}

} // namespace GfxPaint
