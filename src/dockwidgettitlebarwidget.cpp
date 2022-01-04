#include "dockwidgettitlebarwidget.h"

#include <QHBoxLayout>
#include <QApplication>
#include <QStyle>
#include <QAction>
#include <QWidgetAction>
#include <QMenu>
#include <QChar>
#include <QHBoxLayout>

#include "application.h"

namespace GfxPaint {

DockWidgetTitlebarWidget::DockWidgetTitlebarWidget(QWidget *const parent) :
    QWidget(parent),
    titleLabel(new QLabel()), menuToolButton(new QToolButton()), popupToolButton(new QToolButton()),
    m_menuAction(new QAction(this)), m_popupAction(new QWidgetAction(this)), m_floatAction(new QAction(this)), m_closeAction(new QAction(this))
{
    QStyle *const style = qApp->style();

    QHBoxLayout *layout = new QHBoxLayout();
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

    menuToolButton->setVisible(false);
    layout->addWidget(menuToolButton);

    popupToolButton->setVisible(false);
    layout->addWidget(popupToolButton);

    titleLabel->setText("");
    layout->addWidget(titleLabel);

    layout->addStretch();

    QToolButton *const floatToolButton = new QToolButton();
    m_floatAction->setIcon(style->standardIcon(QStyle::SP_TitleBarNormalButton));
    floatToolButton->setDefaultAction(m_floatAction);
    floatToolButton->setAutoRaise(true);
    layout->addWidget(floatToolButton);

    QToolButton *const closeToolButton = new QToolButton();
    m_closeAction->setIcon(style->standardIcon(QStyle::SP_TitleBarCloseButton));
    closeToolButton->setDefaultAction(m_closeAction);
    closeToolButton->setAutoRaise(true);
    layout->addWidget(closeToolButton);

    setLayout(layout);
}

void DockWidgetTitlebarWidget::setMenu(QMenu *const menu) {
    if (menu) {
        menu->menuAction()->setIcon(qApp->style()->standardIcon(QStyle::SP_FileDialogDetailedView));
        menuToolButton->setDefaultAction(menu->menuAction());
        menuToolButton->setPopupMode(QToolButton::InstantPopup);
        popupToolButton->setAutoRaise(true);
        menuToolButton->setVisible(true);
    }
    else {
        menuToolButton->setVisible(false);
        menuToolButton->defaultAction()->deleteLater();
        menuToolButton->setDefaultAction(nullptr);
    }
}

void DockWidgetTitlebarWidget::setPopup(QWidget *const popup) {
    if (popup) {
        QMenu *popupWrapper = new QMenu();
        QHBoxLayout *popupWrapperLayout = new QHBoxLayout();
        popupWrapperLayout->setSpacing(0);
        popupWrapperLayout->setContentsMargins(0, 0, 0, 0);
        popupWrapper->setLayout(popupWrapperLayout);
        popupWrapperLayout->addWidget(popup);
        popupWrapper->menuAction()->setIcon(qApp->style()->standardIcon(QStyle::SP_FileDialogListView));
        popupToolButton->setDefaultAction(popupWrapper->menuAction());
        popupToolButton->setPopupMode(QToolButton::InstantPopup);
        popupToolButton->setAutoRaise(true);
        popupToolButton->setVisible(true);
    }
    else {
        popupToolButton->setVisible(false);
        popupToolButton->defaultAction()->deleteLater();
        popupToolButton->setDefaultAction(nullptr);
    }
}

} // namespace GfxPaint
