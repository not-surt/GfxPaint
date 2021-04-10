#include "sessioneditorwidget.h"
#include "ui_sessioneditorwidget.h"

namespace GfxPaint {

SessionEditorWidget::SessionEditorWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SessionEditorWidget)
{
    ui->setupUi(this);
}

SessionEditorWidget::~SessionEditorWidget()
{
    delete ui;
}

void SessionEditorWidget::setModel(DocumentsModel *const model)
{
    ui->documentsView->setModel(model);
}

QItemSelectionModel *SessionEditorWidget::selectionModel()
{
    return ui->documentsView->selectionModel();
}

} // namespace GfxPaint
