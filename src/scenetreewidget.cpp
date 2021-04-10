#include "scenetreewidget.h"
#include "ui_scenetreewidget.h"

#include "application.h"

namespace GfxPaint {

SceneTreeWidget::SceneTreeWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::SceneTreeWidget),
    editorConnections()
{
    ui->setupUi(this);

    ui->nodesTreeView->setHeaderHidden(true);
    ui->nodesTreeView->setAlternatingRowColors(true);
    ui->nodesTreeView->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->nodesTreeView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->nodesTreeView->setDragEnabled(true);
    //ui->nodesTreeView->setAcceptDrops(true);
    ui->nodesTreeView->setDropIndicatorShown(true);
    //ui->nodesTreeView->setDragDropMode(QAbstractItemView::DragDrop);
    ui->nodesTreeView->setDragDropMode(QAbstractItemView::InternalMove);
    ui->nodesTreeView->setRootIsDecorated(false);
    //ui->nodesTreeView->setItemsExpandable(false);
    ui->nodesTreeView->setUniformRowHeights(true);
    ui->nodesTreeView->setAllColumnsShowFocus(true);
}

SceneTreeWidget::~SceneTreeWidget()
{
    delete ui;
}

void SceneTreeWidget::setEditor(Editor *const editor)
{
    for (const auto &connection : editorConnections) {
        QObject::disconnect(connection);
    }
    editorConnections.clear();
    if (editor) {
        SceneModel *model = qApp->documentManager.documentModel(&editor->scene);
        ui->nodesTreeView->setModel(model);
        ui->nodesTreeView->setSelectionModel(&editor->editingContext().selectionModel());
        ui->nodesTreeView->expandAll();//////////////////////////////////////

        editorConnections << QObject::connect(ui->nodesTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, [this, model](const QItemSelection &selected, const QItemSelection &deselected) {
            QSet<Node *> nodes;
            for (auto index : selected.indexes()) {
                nodes.insert(model->nodeFromIndex(index));
            }
            emit editingNodesChanged(nodes);
        });
    }
    else {
        ui->nodesTreeView->setModel(nullptr);
    }
}

} // namespace GfxPaint
