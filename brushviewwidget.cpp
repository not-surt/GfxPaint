#include "brushviewwidget.h"

#include "application.h"

namespace GfxPaint {

BrushViewWidget::BrushViewWidget(QWidget *const parent) :
    RenderedWidget(parent),
    brush(),
    colour{},
    dabProgram(nullptr)
{
}

BrushViewWidget::~BrushViewWidget()
{
    ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
    delete dabProgram;
}

void BrushViewWidget::setBrush(const Brush &brush)
{
    ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
    delete dabProgram;
    dabProgram = nullptr;

    this->brush = brush;
    update();
}

void BrushViewWidget::render()
{
    if (!dabProgram) dabProgram = new DabProgram(brush.dab.type, brush.dab.metric, widgetBuffer->format(), false, Buffer::Format(), brush.dab.blendMode, brush.dab.composeMode);
    dabProgram->render(brush.dab, colour, viewportTransform, widgetBuffer, nullptr);
}

} // namespace GfxPaint
