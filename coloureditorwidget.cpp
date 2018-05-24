#include "coloureditorwidget.h"
#include "ui_coloureditorwidget.h"

namespace GfxPaint {

ColourEditorWidget::ColourEditorWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ColourEditorWidget),
    m_colour(255, 0, 0, 255)
{
    ui->setupUi(this);

    QObject::connect(ui->colourSpaceComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, &ColourEditorWidget::updateWidgets);
    QObject::connect(ui->alphaCheckBox, &QCheckBox::toggled, this, &ColourEditorWidget::updateWidgets);

    updateWidgets();
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
        updateColour();
    }
}

void ColourEditorWidget::updateColour()
{
    const ColourSpace colourSpace = static_cast<ColourSpace>(ui->colourSpaceComboBox->currentIndex());
    const int componentCount = colourSpaceInfo[colourSpace].componentCount + (ui->alphaCheckBox->isChecked() ? 1 : 0);
    for (int i = 0; i < componentCount; ++i) {
        ColourSliderWidget *const colourSlider = static_cast<ColourSliderWidget *>(ui->colourSliderLayout->itemAt(i)->widget());
        colourSlider->blockSignals(true);
        colourSlider->setColour(m_colour);
        colourSlider->blockSignals(false);
    }
    emit colourChanged(m_colour);
}

void ColourEditorWidget::updateWidgets()
{
    while (!ui->colourSliderLayout->isEmpty()) {
        ui->colourSliderLayout->takeAt(0)->widget()->deleteLater();
    }
    const ColourSpace colourSpace = static_cast<ColourSpace>(ui->colourSpaceComboBox->currentIndex());
    const int componentCount = colourSpaceInfo[colourSpace].componentCount + (ui->alphaCheckBox->isChecked() ? 1 : 0);
    for (int i = 0; i < componentCount; ++i) {
        ColourSliderWidget *const colourSlider = new ColourSliderWidget(colourSpace, i);
        ui->colourSliderLayout->addWidget(colourSlider);
        colourSlider->setColour(m_colour);
        QObject::connect(colourSlider, &ColourSliderWidget::colourChanged, this, &ColourEditorWidget::setColour);
    }

    updateColour();
}

} // namespace GfxPaint
