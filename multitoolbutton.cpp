#include "multitoolbutton.h"

#include <QAction>

namespace GfxPaint {

MultiToolButton::MultiToolButton(QWidget *parent)
    : QToolButton(parent)
{
    setPopupMode(QToolButton::DelayedPopup);
    //setPopupMode(QToolButton::MenuButtonPopup);
    setAutoRaise(true);
    QObject::connect(this, &MultiToolButton::triggered, this, &MultiToolButton::setDefaultAction);
}

} // namespace GfxPaint
