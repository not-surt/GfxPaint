#ifndef TRANSFORMEDITORWIDGET_H
#define TRANSFORMEDITORWIDGET_H

#include <QWidget>

#include <QAbstractTableModel>
#include "editor.h"
#include "transformmodel.h"

namespace GfxPaint {

namespace Ui {
class TransformEditorWidget;
}

class TransformEditorWidget : public QWidget
{
    Q_OBJECT

public:
    explicit TransformEditorWidget(QWidget *parent = 0);
    ~TransformEditorWidget();

    const QTransform &transform() const;
    void setTransform(const QTransform &transform);

    TransformMode transformMode() const;
    void setTransformMode(const TransformMode transformMode);

signals:
    void transformChanged(const QTransform &transform);
    void transformModeChanged(const TransformMode transformMode);

private:
    Ui::TransformEditorWidget *ui;

    TransformModel model;
    TransformMode m_transformMode;
};

} // namespace GfxPaint

#endif // TRANSFORMEDITORWIDGET_H
