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
    QList<Program *> oldPrograms = {program};
    program = new DabProgram(brush.dab.type, brush.dab.metric, RenderedWidget::format, false, Buffer::Format(), brush.dab.blendMode, brush.dab.composeMode);
    qDeleteAll(oldPrograms);

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
    program->render(brush.dab, colour, viewportTransform, widgetBuffer, nullptr);
}

} // namespace GfxPaint
