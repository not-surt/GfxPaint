#include "brushviewwidget.h"

#include "application.h"

namespace GfxPaint {

BrushViewWidget::BrushViewWidget(QWidget *const parent) :
    RenderedWidget(parent),
    brush(),
    colour{},
    program(nullptr)
{
}

BrushViewWidget::~BrushViewWidget()
{
    ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
    delete program;
}

void BrushViewWidget::setBrush(const Brush &brush)
{
    ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
    std::list<Program *> oldPrograms = {program};
    program = new BrushDabProgram(brush.dab.type, brush.dab.metric, RenderedWidget::format, false, Buffer::Format(), 0, RenderManager::composeModeDefault);
    oldPrograms.clear();

    this->brush = brush;
    update();
}

void BrushViewWidget::initializeGL()
{
    RenderedWidget::initializeGL();

    setBrush(brush);
}

void BrushViewWidget::render()
{
    program->render({Stroke::Point({0.0, 0.0}, 0.0, {}, 0.0, 0.0)}, brush.dab, colour, Mat4(), viewportTransform, widgetBuffer, nullptr);
}

} // namespace GfxPaint
