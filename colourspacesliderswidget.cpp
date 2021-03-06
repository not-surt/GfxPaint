#include "colourspacesliderswidget.h"
#include "ui_colourspacesliderswidget.h"

#include "application.h"

namespace GfxPaint {

ColourSpaceSlidersWidget::ColourSpaceSlidersWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ColourSpaceSlidersWidget),
    fromRGBConversionProgram(nullptr), toRGBConversionProgram(nullptr),
    m_colour(255, 0, 0, 255),
    m_palette(nullptr)
{
    ui->setupUi(this);

    QObject::connect(ui->colourSpaceComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, &ColourSpaceSlidersWidget::updateWidgets);
    QObject::connect(ui->alphaCheckBox, &QCheckBox::toggled, this, &ColourSpaceSlidersWidget::updateWidgets);
    QObject::connect(ui->quantiseCheckBox, &QCheckBox::toggled, this, &ColourSpaceSlidersWidget::updateWidgets);

    updateWidgets();
}

ColourSpaceSlidersWidget::~ColourSpaceSlidersWidget()
{
    delete ui;

    {
        ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
        delete fromRGBConversionProgram;
        delete toRGBConversionProgram;
    }
}

QColor ColourSpaceSlidersWidget::colour() const
{
    return m_colour;
}

void ColourSpaceSlidersWidget::setColour(const QColor &colour)
{
    if (m_colour != colour) {
        m_colour = colour;
        updateColour();
    }
}

void ColourSpaceSlidersWidget::setPalette(const Buffer *const palette)
{
    if (m_palette != palette) {
        m_palette = palette;
        if (ui->quantiseCheckBox->isChecked()) updateWidgets();
    }
}

void ColourSpaceSlidersWidget::updateColour()
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

void ColourSpaceSlidersWidget::updateWidgets()
{
    while (ui->colourSliderLayout->count() > 0) {
        ui->colourSliderLayout->takeAt(0)->widget()->deleteLater();
    }
    const ColourSpace colourSpace = static_cast<ColourSpace>(ui->colourSpaceComboBox->currentIndex());
    {
        const QList<Program *> oldPrograms = {fromRGBConversionProgram, toRGBConversionProgram};
        ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
        fromRGBConversionProgram = new ColourConversionProgram(ColourSpace::RGB, colourSpace);
        toRGBConversionProgram = new ColourConversionProgram(colourSpace, ColourSpace::RGB);
        qDeleteAll(oldPrograms);
    }
    const int componentCount = colourSpaceInfo[colourSpace].componentCount + (ui->alphaCheckBox->isChecked() ? 1 : 0);
    for (int i = 0; i < componentCount; ++i) {
        ColourSliderWidget *const colourSlider = new ColourSliderWidget(colourSpace, i, ui->quantiseCheckBox->isChecked() && m_palette, m_palette ? m_palette->format() : Buffer::Format());
        ui->colourSliderLayout->addWidget(colourSlider);
        colourSlider->setColour(m_colour);
        colourSlider->setPalette(m_palette);
        QObject::connect(colourSlider, &ColourSliderWidget::colourChanged, this, &ColourSpaceSlidersWidget::setColour);
        QObject::connect(colourSlider, &ColourSliderWidget::posChanged, [this](const GLfloat pos){
//            setColour(toRGBConversionProgram->convert());
        });
    }

    updateColour();
}

} // namespace GfxPaint
