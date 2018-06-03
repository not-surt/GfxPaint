#ifndef COLOURSPACESLIDERSWIDGET_H
#define COLOURSPACESLIDERSWIDGET_H

#include <QWidget>

#include "coloursliderwidget.h"

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

    QColor colour() const;

public slots:
    void setColour(const QColor &colour);
    void setPalette(const Buffer *const palette);

protected slots:
    void updateColour();

signals:
    void colourChanged(const QColor &colour);

private:
    void updateWidgets();

    Ui::ColourSpaceSlidersWidget *ui;

    ColourConversionProgram *fromRGBConversionProgram;
    ColourConversionProgram *toRGBConversionProgram;

    QColor m_colour;
    const Buffer *m_palette;
};


} // namespace GfxPaint
#endif // COLOURSPACESLIDERSWIDGET_H
