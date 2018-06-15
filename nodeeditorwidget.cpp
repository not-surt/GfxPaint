#include "nodeeditorwidget.h"
#include "ui_nodeeditorwidget.h"

namespace GfXPaint {

NodeEditorWidget::NodeEditorWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::NodeEditorWidget)
{
    ui->setupUi(this);
}

NodeEditorWidget::~NodeEditorWidget()
{
    delete ui;
}

} // namespace GfXPaint
