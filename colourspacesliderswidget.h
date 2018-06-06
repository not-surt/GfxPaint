#ifndef COLOURSPACESLIDERSWIDGET_H
#define COLOURSPACESLIDERSWIDGET_H

#include <QWidget>

#include "colourcomponentsliderwidget.h"

namespace GfxPaint {

namespace Ui {
class ColourSpaceSlidersWidget;
}

class ColourSpaceSlidersWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ColourSpaceSlidersWidget(QWidget *parent = nullptr);
    ~ColourSpaceSlidersWidget();

    Colour colour() const;

public slots:
    void setColour(const Colour &colour);
    void setPalette(const Buffer *const palette);

signals:
    void colourChanged(const Colour &colour);

private:
    void updateSliderColours();
    void updateSliderPositions();
    void updateColourFromSliders();
    void updateWidgets();

    Ui::ColourSpaceSlidersWidget *ui;

    ColourConversionProgram *fromRGBConversionProgram;
    ColourConversionProgram *toRGBConversionProgram;

    Colour m_colour;
    const Buffer *m_palette;
};

} // namespace GfxPaint

#endif // COLOURSPACESLIDERSWIDGET_H
