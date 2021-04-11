#include "paletteeditorwidget.h"
#include "ui_paletteeditorwidget.h"

namespace GfxPaint {

PaletteEditorWidget::PaletteEditorWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PaletteEditorWidget)
{
    ui->setupUi(this);

    QObject::connect(ui->colourPaletteWidget, &ColourPaletteWidget::colourPicked, this, &PaletteEditorWidget::colourPicked);
}

PaletteEditorWidget::~PaletteEditorWidget()
{
    delete ui;
}

void PaletteEditorWidget::setPalette(const Buffer *const palette)
{
    ui->colourPaletteWidget->setPalette(palette);
}

} // namespace GfxPaint
