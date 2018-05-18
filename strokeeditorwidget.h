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
    explicit StrokeEditorWidget(QWidget *parent = 0);
    ~StrokeEditorWidget();

public slots:
    void setStroke(const Stroke &stroke);

protected slots:
    void updateStroke();

signals:
    void strokeChanged(const Stroke &stroke);

private:
    void updateWidgets();

    Ui::StrokeEditorWidget *ui;

    Stroke stroke;
};

} // namespace GfxPaint

#endif // STROKEEDITORWIDGET_H
