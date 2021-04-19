#include "coloursliderswidget.h"
#include "ui_coloursliderswidget.h"

#include "colourcomponentsplanewidget.h"
#include "application.h"
#include "utils.h"

namespace GfxPaint {

ColourSlidersWidget::ColourSlidersWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ColourSlidersWidget),
    fromRGBConversionProgram(nullptr), toRGBConversionProgram(nullptr),
    m_colour{},
    m_palette(nullptr)
{
    ui->setupUi(this);

    ui->colourSpaceComboBox->blockSignals(true);
    ui->colourSpaceComboBox->clear();
    auto iterator = colourSpaceInfo.begin();
    while (iterator != colourSpaceInfo.end()) {
        ui->colourSpaceComboBox->insertItem(static_cast<int>(iterator.key()), iterator.value().label);
        ++iterator;
    }
    ui->colourSpaceComboBox->setCurrentIndex(0);
    ui->colourSpaceComboBox->blockSignals(false);

    QObject::connect(ui->colourSpaceComboBox, qOverload<int>(&QComboBox::activated), this, &ColourSlidersWidget::updateColourSpace);
    QObject::connect(ui->alphaCheckBox, &QCheckBox::toggled, this, &ColourSlidersWidget::updateWidgets);
    QObject::connect(ui->quantiseCheckBox, &QCheckBox::toggled, this, &ColourSlidersWidget::updateWidgets);

    setColour(m_colour);
    updateColourSpace();
}

ColourSlidersWidget::~ColourSlidersWidget()
{
    delete ui;

    {
        ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
        delete fromRGBConversionProgram;
        delete toRGBConversionProgram;
    }
}

Colour ColourSlidersWidget::colour() const
{
    return m_colour;
}

void ColourSlidersWidget::setColour(const Colour &colour)
{
    if (m_colour != colour) {
        m_colour = colour;
        updateSliderColours();
        updateSliderPositions();
        emit colourChanged(m_colour);
    }
}

void ColourSlidersWidget::setPalette(const Buffer *const palette)
{
    if (m_palette != palette) {
        m_palette = palette;
        if (ui->quantiseCheckBox->isChecked()) updateWidgets();
    }
}

void ColourSlidersWidget::updateSliderColours()
{
    const ColourSpace &colourSpace = static_cast<ColourSpace>(ui->colourSpaceComboBox->currentIndex());

    const int componentCount = colourSpaceInfo[colourSpace].componentCount + (ui->alphaCheckBox->isChecked() ? 1 : 0);
    for (int i = 0; i < componentCount; ++i) {
        ColourComponentsPlaneWidget *const colourSlider = static_cast<ColourComponentsPlaneWidget *>(ui->colourSliderLayout->itemAt(i)->widget());
        colourSlider->blockSignals(true);
        colourSlider->setColour(m_colour);
        colourSlider->blockSignals(false);
    }
}

void ColourSlidersWidget::updateSliderPositions()
{
    const ColourSpace &colourSpace = static_cast<ColourSpace>(ui->colourSpaceComboBox->currentIndex());

    const int componentCount = colourSpaceInfo[colourSpace].componentCount + (ui->alphaCheckBox->isChecked() ? 1 : 0);
    Colour spaceColour = colourSpace != ColourSpace::RGB ? fromRGBConversionProgram->convert(m_colour) : m_colour;
    for (int i = 0; i < componentCount; ++i) {
        ColourComponentsPlaneWidget *const colourSlider = static_cast<ColourComponentsPlaneWidget *>(ui->colourSliderLayout->itemAt(i)->widget());
        colourSlider->blockSignals(true);
        colourSlider->setPos(QVector2D(clamp(0.0f, 1.0f, spaceColour.rgba[i]), 0.0f));
        colourSlider->blockSignals(false);
    }
}

void ColourSlidersWidget::updateColourFromSliders()
{
    const ColourSpace &colourSpace = static_cast<ColourSpace>(ui->colourSpaceComboBox->currentIndex());

    const int componentCount = colourSpaceInfo[colourSpace].componentCount + (ui->alphaCheckBox->isChecked() ? 1 : 0);
    Colour spaceColour = m_colour;
    for (int i = 0; i < componentCount; ++i) {
        ColourComponentsPlaneWidget *const colourSlider = static_cast<ColourComponentsPlaneWidget *>(ui->colourSliderLayout->itemAt(i)->widget());
        spaceColour.rgba[i] = clamp(0.0f, 1.0f, colourSlider->pos().x());
    }
    m_colour = colourSpace != ColourSpace::RGB ? toRGBConversionProgram->convert(spaceColour) : spaceColour;
}

void ColourSlidersWidget::updateWidgets()
{
    const ColourSpace &colourSpace = static_cast<ColourSpace>(ui->colourSpaceComboBox->currentIndex());

    while (ui->colourSliderLayout->count() > 0) {
        ui->colourSliderLayout->takeAt(0)->widget()->deleteLater();
    }
    const int componentCount = colourSpaceInfo[colourSpace].componentCount + (ui->alphaCheckBox->isChecked() ? 1 : 0);
    for (int i = 0; i < componentCount; ++i) {
        ColourComponentsPlaneWidget *const colourSlider = new ColourComponentsPlaneWidget(colourSpace, i, -1, ui->quantiseCheckBox->isChecked() && m_palette, m_palette ? m_palette->format() : Buffer::Format());
        ui->colourSliderLayout->addWidget(colourSlider);
        colourSlider->setPalette(m_palette);
        QObject::connect(colourSlider, &ColourComponentsPlaneWidget::posChanged, [this](){
            updateColourFromSliders();
            updateSliderColours();
            emit colourChanged(m_colour);
        });
    }
    updateSliderPositions();
    updateSliderColours();
}

void ColourSlidersWidget::updateColourSpace()
{
    const ColourSpace &colourSpace = static_cast<ColourSpace>(ui->colourSpaceComboBox->currentIndex());

    {
        const QList<Program *> oldPrograms = {fromRGBConversionProgram, toRGBConversionProgram};
        ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
        fromRGBConversionProgram = new ColourConversionProgram(ColourSpace::RGB, colourSpace);
        toRGBConversionProgram = new ColourConversionProgram(colourSpace, ColourSpace::RGB);
        qDeleteAll(oldPrograms);
    }

    updateWidgets();
}

} // namespace GfxPaint
