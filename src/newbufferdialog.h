#ifndef NEWBUFFERDIALOG_H
#define NEWBUFFERDIALOG_H

#include "ui_newbufferdialog.h"

#include <QDialog>

#include "buffer.h"

namespace GfxPaint {

namespace Ui {
class NewBufferDialog;
}

class NewBufferDialog : public QDialog
{
    Q_OBJECT

public:
    explicit NewBufferDialog(QWidget *parent = nullptr, Qt::WindowFlags flags = Qt::WindowFlags());
    virtual ~NewBufferDialog();

    QSize imageSize() const {
        return QSize(ui->imageWidthSpinBox->value(), ui->imageHeightSpinBox->value());
    }
    Buffer::Format format() const {
        return Buffer::Format(static_cast<Buffer::Format::ComponentType>(ui->componentTypeComboBox->currentData().toInt()), ui->componentSizeComboBox->currentData().toInt(), ui->componentCountComboBox->currentData().toInt());
    }
    QSizeF pixelRatio() const {
        return QSizeF(ui->pixelWidthSpinBox->value(), ui->pixelHeightSpinBox->value());
    }
    bool indexed() const {
        return ui->indexedCheckBox->isChecked();
    }
    int blendMode() const {
        return ui->blendModeComboBox->currentIndex();
    }
    int composeMode() const {
        return ui->composeModeComboBox->currentIndex();
    }
    qreal opacity() const {
        return ui->opacitySpinBox->value();
    }

protected:
    virtual void hideEvent(QHideEvent *event) override;

private:
    Ui::NewBufferDialog *const ui;
};

} // namespace GfxPaint

#endif // NEWBUFFERDIALOG_H
