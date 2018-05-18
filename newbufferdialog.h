#ifndef NEWBUFFERDIALOG_H
#define NEWBUFFERDIALOG_H

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

    QSize imageSize() const;
    Buffer::Format format() const;
    QSizeF pixelRatio() const;

protected:
    virtual void hideEvent(QHideEvent *event) override;

private:
    Ui::NewBufferDialog *const ui;
};

} // namespace GfxPaint

#endif // NEWBUFFERDIALOG_H
