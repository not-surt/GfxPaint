#include "coloureditorwidget.h"
#include "ui_coloureditorwidget.h"

namespace GfxPaint {

ColourEditorWidget::ColourEditorWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ColourEditorWidget),
    m_colour(255, 0, 0, 255)
{
    ui->setupUi(this);

    QObject::connect(ui->colourSliderWidget, &ColourSliderWidget::colourChanged, this, &ColourEditorWidget::setColour);
}

ColourEditorWidget::~ColourEditorWidget()
{
    delete ui;
}

QColor ColourEditorWidget::colour() const
{
    return m_colour;
}

void ColourEditorWidget::setColour(const QColor &colour)
{
    if (m_colour != colour) {
        m_colour = colour;
        ui->colourSliderWidget->setColour(colour);
        emit colourChanged(colour);
        update();
    }
}

} // namespace GfxPaint
