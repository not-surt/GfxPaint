#ifndef SCENEGRAPHWIDGET_H
#define SCENEGRAPHWIDGET_H

#include <QWidget>

#include <QItemSelectionModel>
#include "editor.h"

namespace GfxPaint {

namespace Ui {
class SceneTreeWidget;
}

class SceneTreeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit SceneTreeWidget(QWidget *parent = 0);
    ~SceneTreeWidget();

    void setEditor(Editor *const editor);

signals:
    void editingNodesChanged(QSet<Node *> nodes);

private:
    Ui::SceneTreeWidget *ui;
    QList<QMetaObject::Connection> editorConnections;
};

} // namespace GfxPaint

#endif // SCENEGRAPHWIDGET_H
