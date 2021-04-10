#include "dockwidgettitlebarwidget.h"

#include <QHBoxLayout>
#include <QApplication>
#include <QStyle>
#include <QAction>
#include <QWidgetAction>
#include <QMenu>

namespace GfxPaint {

DockWidgetTitlebarWidget::DockWidgetTitlebarWidget(QWidget *const parent) :
    QWidget(parent),
    titleLabel(new QLabel()),
    m_menuAction(new QAction(this)), m_popupAction(new QWidgetAction(this)), m_floatAction(new QAction(this)), m_closeAction(new QAction(this))
{
    QStyle *const style = qApp->style();

    QHBoxLayout *layout = new QHBoxLayout();
    layout->setSpacing(0);
    layout->setContentsMargins(0, 0, 0, 0);

    m_menuAction->setIcon(style->standardIcon(QStyle::SP_FileDialogDetailedView));
    QToolButton *const menuToolButton = new QToolButton();
    menuToolButton->setDefaultAction(m_menuAction);
    menuToolButton->setPopupMode(QToolButton::InstantPopup);
//    menuToolButton->setArrowType(Qt::ArrowType::NoArrow);
    menuToolButton->setAutoRaise(true);
    layout->addWidget(menuToolButton);
    QObject::connect(m_popupAction, &QAction::changed, this, [this, menuToolButton](){
        menuToolButton->setVisible(m_menuAction->isEnabled());
    });

    m_popupAction->setDefaultWidget(new QWidget(this));
    m_popupAction->setIcon(style->standardIcon(QStyle::SP_FileDialogListView));
    QToolButton *const popupToolButton = new QToolButton();
    popupToolButton->setDefaultAction(m_popupAction);
    popupToolButton->addAction(m_popupAction);
    popupToolButton->setPopupMode(QToolButton::InstantPopup);
//    popupToolButton->setArrowType(Qt::ArrowType::DownArrow);
    popupToolButton->setAutoRaise(true);
    layout->addWidget(popupToolButton);
    QObject::connect(m_popupAction, &QAction::changed, this, [this, popupToolButton](){
        popupToolButton->setVisible(m_popupAction->isEnabled());
    });

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

} // namespace GfxPaint
