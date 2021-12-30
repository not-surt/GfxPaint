#ifndef NODEEDITORWIDGET_H
#define NODEEDITORWIDGET_H

#include <QWidget>

namespace GfXPaint {

namespace Ui {
class NodeEditorWidget;
}

class NodeEditorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit NodeEditorWidget(QWidget *parent = 0);
    ~NodeEditorWidget();

private:
    Ui::NodeEditorWidget *ui;
};

} // namespace GfXPaint

#endif // NODEEDITORWIDGET_H
