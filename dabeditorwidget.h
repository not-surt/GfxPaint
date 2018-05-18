#ifndef DABEDITORWIDGET_H
#define DABEDITORWIDGET_H

#include <QWidget>

#include "brush.h"

namespace GfxPaint {

namespace Ui {
class DabEditorWidget;
}

class DabEditorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit DabEditorWidget(QWidget *parent = 0);
    ~DabEditorWidget();

public slots:
    void setDab(const Dab &dab);

protected slots:
    void updateDab();
    void updateRatio(const QString &string);

signals:
    void dabChanged(const Dab &dab);

private:
    void updateWidgets();

    Ui::DabEditorWidget *ui;

    Dab dab;
};

} // namespace GfxPaint

#endif // DABEDITORWIDGET_H
