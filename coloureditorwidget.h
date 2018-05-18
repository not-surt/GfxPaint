#ifndef COLOUREDITORWIDGET_H
#define COLOUREDITORWIDGET_H

#include <QWidget>

namespace GfxPaint {

namespace Ui {
class ColourEditorWidget;
}

class ColourEditorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit ColourEditorWidget(QWidget *parent = 0);
    ~ColourEditorWidget();

    QColor colour() const;

public slots:
    void setColour(const QColor &colour);

signals:
    void colourChanged(const QColor &colour);

private:
    Ui::ColourEditorWidget *ui;

    QColor m_colour;
};


} // namespace GfxPaint
#endif // COLOUREDITORWIDGET_H
