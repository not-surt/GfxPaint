#ifndef SESSIONEDITORWIDGET_H
#define SESSIONEDITORWIDGET_H

#include <QWidget>

#include <QItemSelectionModel>
#include "documentsmodel.h"

namespace GfxPaint {

namespace Ui {
class SessionEditorWidget;
}

class SessionEditorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SessionEditorWidget(QWidget *parent = 0);
    ~SessionEditorWidget();

    void setModel(DocumentsModel *const model);
    QItemSelectionModel *selectionModel();

private:
    Ui::SessionEditorWidget *ui;
};


} // namespace GfxPaint
#endif // SESSIONEDITORWIDGET_H
