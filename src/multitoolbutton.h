#ifndef MULTITOOLBUTTON_H
#define MULTITOOLBUTTON_H

#include <QToolButton>

namespace GfxPaint {

class MultiToolButton : public QToolButton
{
    Q_OBJECT

public:
    explicit MultiToolButton(QWidget *parent = nullptr);
};

} // namespace GfxPaint

#endif // MULTITOOLBUTTON_H
