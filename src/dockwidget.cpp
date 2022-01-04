#include "dockwidget.h"

#include <QMenu>
#include <QDebug>
#include <QHBoxLayout>
#include <QPushButton>

namespace GfxPaint {

DockWidget::DockWidget(QWidget *const parent, const Qt::WindowFlags &flags) :
    QDockWidget(parent, flags),
    titleBar(new DockWidgetTitlebarWidget(this))
{
    setTitleBarWidget(titleBar);

    QObject::connect(this, &QDockWidget::windowTitleChanged, titleBar, &DockWidgetTitlebarWidget::setTitle);

    QMenu *menu = new QMenu(this);
    menu->addAction(new QAction("Poop!", this));
    menu->addAction(new QAction("Scoop!", this));
    setMenu(menu);

    QWidget *popup = new QWidget(this);
    popup->setLayout(new QHBoxLayout());
    popup->layout()->addWidget(new QPushButton());
    popup->layout()->addWidget(new QPushButton());
    popup->layout()->addWidget(new QPushButton());
    popup->layout()->addWidget(new QPushButton());
    popup->layout()->addWidget(new QPushButton());
    setPopup(popup);

    QObject::connect(titleBar->floatAction(), &QAction::triggered, this, [this](){
        this->setFloating(!isFloating());
    });
    QObject::connect(titleBar->closeAction(), &QAction::triggered, this, [this](){
        this->setVisible(!isVisible());
    });
}

void DockWidget::setMenu(QMenu *const menu) {
    titleBar->setMenu(menu);
}

void DockWidget::setPopup(QWidget *const popup) {
    titleBar->setPopup(popup);
}

} // namespace GfxPaint
