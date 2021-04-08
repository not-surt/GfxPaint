#ifndef COLOURSPACEPLANEWIDGET_H
#define COLOURSPACEPLANEWIDGET_H

#include <QWidget>

#include "colourcomponentsliderwidget.h"

namespace GfxPaint {

namespace Ui {
class ColourSpacePlaneWidget;
}
class ColourComponentsPlaneWidget;
class ColourComponentSliderWidget;

class ColourSpacePlaneWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ColourSpacePlaneWidget(QWidget *parent = nullptr);
    ~ColourSpacePlaneWidget();

    Colour colour() const;

public slots:
    void setColour(const GfxPaint::Colour &colour);
    void setPalette(const GfxPaint::Buffer *const palette);

signals:
    void colourChanged(const GfxPaint::Colour &colour);

private:
    void updateWidgetColours();
    void updateWidgetPositions();
    void updateColourFromWidgets();
    void updateWidgets();

    Ui::ColourSpacePlaneWidget *ui;
    ColourComponentsPlaneWidget *plane;
    ColourComponentSliderWidget *alphaSlider;

    ColourConversionProgram *fromRGBConversionProgram;
    ColourConversionProgram *toRGBConversionProgram;

    Colour m_colour;
    const Buffer *m_palette;
};

} // namespace GfxPaint

#endif // COLOURSPACEPLANEWIDGET_H
