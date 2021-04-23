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
    explicit TransformEditorWidget(QWidget *parent = nullptr);
    ~TransformEditorWidget();

    const Mat4 &transform() const;
    void setTransform(const Mat4 &transform);

    TransformTarget transformMode() const;
    void setTransformMode(const TransformTarget transformMode);

signals:
    void transformChanged(const Mat4 &transform);
    void transformModeChanged(const GfxPaint::TransformTarget transformMode);

private:
    Ui::TransformEditorWidget *ui;

    TransformModel model;
    TransformTarget m_transformMode;
};

} // namespace GfxPaint

#endif // TRANSFORMEDITORWIDGET_H
