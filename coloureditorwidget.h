#ifndef COLOUREDITORWIDGET_H
#define COLOUREDITORWIDGET_H

#include <QWidget>

#include "coloursliderwidget.h"

namespace GfxPaint {

namespace Ui {
class ColourEditorWidget;
}

class ColourEditorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ColourEditorWidget(QWidget *parent = nullptr);
    ~ColourEditorWidget();

    QColor colour() const;

public slots:
    void setColour(const QColor &colour);

protected slots:
    void updateColour();

signals:
    void colourChanged(const QColor &colour);

private:
    void updateWidgets();

    Ui::ColourEditorWidget *ui;

    QColor m_colour;
};


} // namespace GfxPaint
#endif // COLOUREDITORWIDGET_H
