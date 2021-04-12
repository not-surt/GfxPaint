#ifndef STROKEEDITORWIDGET_H
#define STROKEEDITORWIDGET_H

#include <QWidget>

#include "brush.h"

namespace GfxPaint {

namespace Ui {
class StrokeEditorWidget;
}

class StrokeEditorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit StrokeEditorWidget(QWidget *parent = nullptr);
    ~StrokeEditorWidget();

public slots:
    void setStroke(const Brush::Stroke &stroke);

protected slots:
    void updateStroke();

signals:
    void strokeChanged(const Brush::Stroke &stroke);

private:
    void updateWidgets();

    Ui::StrokeEditorWidget *ui;

    Brush::Stroke stroke;
};

} // namespace GfxPaint

#endif // STROKEEDITORWIDGET_H
