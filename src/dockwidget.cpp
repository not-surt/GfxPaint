#include "dockwidget.h"

#include <QMenu>
#include <QDebug>
#include <QHBoxLayout>
#include <QPushButton>

namespace GfxPaint {

DockWidget::DockWidget(QWidget *const parent, const Qt::WindowFlags &flags) :
    QDockWidget(parent, flags),
    titleBar(new DockWidgetTitlebarWidget(this)), m_menu(nullptr), m_popup(nullptr)
{
    setTitleBarWidget(titleBar);

    QObject::connect(this, &QDockWidget::windowTitleChanged, titleBar, &DockWidgetTitlebarWidget::setTitle);

    m_menu = new QMenu(this);
    m_menu->addAction(new QAction("Poop!", this));
    m_menu->addAction(new QAction("Scoop!", this));
    setMenu(m_menu);

    m_popup = new QWidget(this);
    m_popup->setLayout(new QHBoxLayout());
    m_popup->layout()->addWidget(new QPushButton());
    m_popup->layout()->addWidget(new QPushButton());
    m_popup->layout()->addWidget(new QPushButton());
    m_popup->layout()->addWidget(new QPushButton());
    m_popup->layout()->addWidget(new QPushButton());
    setPopup(m_popup);

    QObject::connect(titleBar->floatAction(), &QAction::triggered, this, [this](){
        this->setFloating(!isFloating());
    });
    QObject::connect(titleBar->closeAction(), &QAction::triggered, this, [this](){
        this->setVisible(!isVisible());
    });
}

} // namespace GfxPaint
