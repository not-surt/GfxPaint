#include "colourspaceplanewidget.h"
#include "ui_colourspaceplanewidget.h"

#include "colourcomponentsplanewidget.h"
#include "application.h"
#include "utils.h"

namespace GfxPaint {

ColourSpacePlaneWidget::ColourSpacePlaneWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ColourSpacePlaneWidget), plane(nullptr), zComponentSlider(nullptr), alphaSlider(nullptr),
    fromRGBConversionProgram(nullptr), toRGBConversionProgram(nullptr),
    m_colour{},
    m_palette(nullptr)
{
    ui->setupUi(this);

    setColour(m_colour);
    updateWidgets();

    QObject::connect(ui->colourSpaceComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, &ColourSpacePlaneWidget::updateWidgets);
    QObject::connect(ui->xComponentComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, &ColourSpacePlaneWidget::updateWidgets);
    QObject::connect(ui->yComponentComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, &ColourSpacePlaneWidget::updateWidgets);
    QObject::connect(ui->zComponentComboBox, qOverload<int>(&QComboBox::currentIndexChanged), this, &ColourSpacePlaneWidget::updateWidgets);
    QObject::connect(ui->alphaCheckBox, &QCheckBox::toggled, this, &ColourSpacePlaneWidget::updateWidgets);
    QObject::connect(ui->quantiseCheckBox, &QCheckBox::toggled, this, &ColourSpacePlaneWidget::updateWidgets);
}

ColourSpacePlaneWidget::~ColourSpacePlaneWidget()
{
    delete ui;

    {
        ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
        delete fromRGBConversionProgram;
        delete toRGBConversionProgram;
    }
}

Colour ColourSpacePlaneWidget::colour() const
{
    return m_colour;
}

void ColourSpacePlaneWidget::setColour(const Colour &colour)
{
    if (m_colour != colour) {
        m_colour = colour;
        updateWidgetColours();
        updateWidgetPositions();
        emit colourChanged(m_colour);
    }
}

void ColourSpacePlaneWidget::setPalette(const Buffer *const palette)
{
    if (m_palette != palette) {
        m_palette = palette;
        if (ui->quantiseCheckBox->isChecked()) updateWidgets();
    }
}

void ColourSpacePlaneWidget::updateWidgetColours()
{
    plane->blockSignals(true);
    plane->setColour(m_colour);
    plane->blockSignals(false);
    zComponentSlider->blockSignals(true);
    zComponentSlider->setColour(m_colour);
    zComponentSlider->blockSignals(false);
    if (ui->alphaCheckBox->isChecked()) {
        alphaSlider->blockSignals(true);
        alphaSlider->setColour(m_colour);
        alphaSlider->blockSignals(false);
    }
}

void ColourSpacePlaneWidget::updateWidgetPositions()
{
    const ColourSpace colourSpace = static_cast<ColourSpace>(ui->colourSpaceComboBox->currentIndex());
    Colour spaceColour = fromRGBConversionProgram->convert(m_colour);
    plane->blockSignals(true);
    plane->setPos(QVector2D(spaceColour.rgba[ui->xComponentComboBox->currentIndex()], spaceColour.rgba[ui->yComponentComboBox->currentIndex()]));
    plane->blockSignals(false);
    zComponentSlider->blockSignals(true);
    zComponentSlider->setPos(spaceColour.rgba[ui->zComponentComboBox->currentIndex()]);
    zComponentSlider->blockSignals(false);
    if (ui->alphaCheckBox->isChecked()) {
        int alphaComponent = colourSpaceInfo[colourSpace].componentCount;
        alphaSlider->blockSignals(true);
        alphaSlider->setPos(clamp(0.0f, 1.0f, spaceColour.rgba[alphaComponent]));
        alphaSlider->blockSignals(false);
    }
}

void ColourSpacePlaneWidget::updateColourFromWidgets()
{
    const ColourSpace colourSpace = static_cast<ColourSpace>(ui->colourSpaceComboBox->currentIndex());
    Colour spaceColour = m_colour;
    spaceColour.rgba[ui->xComponentComboBox->currentIndex()] = clamp(0.0f, 1.0f, plane->pos().x());
    spaceColour.rgba[ui->yComponentComboBox->currentIndex()] = clamp(0.0f, 1.0f, plane->pos().y());
    spaceColour.rgba[ui->zComponentComboBox->currentIndex()] = clamp(0.0f, 1.0f, (float)zComponentSlider->pos());
    if (ui->alphaCheckBox->isChecked()) {
        int alphaComponent = colourSpaceInfo[colourSpace].componentCount;
        spaceColour.rgba[alphaComponent] = clamp(0.0f, 1.0f, (float)alphaSlider->pos());
    }
    m_colour = toRGBConversionProgram->convert(spaceColour);
}

void ColourSpacePlaneWidget::updateWidgets()
{
    while (ui->colourSpacePlaneLayout->count() > 0) {
        ui->colourSpacePlaneLayout->takeAt(0)->widget()->deleteLater();
    }
    const ColourSpace colourSpace = static_cast<ColourSpace>(ui->colourSpaceComboBox->currentIndex());
    {
        const QList<Program *> oldPrograms = {fromRGBConversionProgram, toRGBConversionProgram};
        ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
        fromRGBConversionProgram = new ColourConversionProgram(ColourSpace::RGB, colourSpace);
        toRGBConversionProgram = new ColourConversionProgram(colourSpace, ColourSpace::RGB);
        qDeleteAll(oldPrograms);
    }
    QVector<QComboBox *> comboBoxes = {ui->xComponentComboBox, ui->yComponentComboBox, ui->zComponentComboBox};
    for (int i = 0; i < comboBoxes.length(); ++i) {
        QComboBox *const comboBox = comboBoxes[i];
        const int componentIndex = (comboBox->currentIndex() >= 0 ? comboBox->currentIndex() : i);
        comboBox->blockSignals(true);
        comboBox->clear();
        for (int i = 0; i < colourSpaceInfo[colourSpace].componentCount; ++i) {
            comboBox->addItem(colourSpaceInfo[colourSpace].componentLabels[i], i);
        }
        comboBox->setCurrentIndex(componentIndex);
        comboBox->blockSignals(false);
    }
    auto updateFromChildWidget = [this](){
        updateColourFromWidgets();
        updateWidgetColours();
        emit colourChanged(m_colour);
    };
    plane = new ColourComponentsPlaneWidget(colourSpace, ui->xComponentComboBox->currentIndex(), ui->yComponentComboBox->currentIndex(), ui->quantiseCheckBox->isChecked() && m_palette, m_palette ? m_palette->format() : Buffer::Format());
    ui->colourSpacePlaneLayout->addWidget(plane);
    plane->setPalette(m_palette);
    QObject::connect(plane, &ColourComponentsPlaneWidget::posChanged, updateFromChildWidget);
    zComponentSlider = new ColourComponentSliderWidget(colourSpace, ui->zComponentComboBox->currentIndex(), ui->quantiseCheckBox->isChecked() && m_palette, m_palette ? m_palette->format() : Buffer::Format());
    ui->colourSpacePlaneLayout->addWidget(zComponentSlider);
    zComponentSlider->setPalette(m_palette);
    QObject::connect(zComponentSlider, &ColourComponentSliderWidget::posChanged, updateFromChildWidget);
    if (ui->alphaCheckBox->isChecked()) {
        int alphaComponent = colourSpaceInfo[colourSpace].componentCount;
        alphaSlider = new ColourComponentSliderWidget(colourSpace, alphaComponent, ui->quantiseCheckBox->isChecked() && m_palette, m_palette ? m_palette->format() : Buffer::Format());
        ui->colourSpacePlaneLayout->addWidget(alphaSlider);
        alphaSlider->setPalette(m_palette);
        QObject::connect(alphaSlider, &ColourComponentSliderWidget::posChanged, updateFromChildWidget);
    }
    updateWidgetPositions();
    updateWidgetColours();
}

} // namespace GfxPaint
