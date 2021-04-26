#ifndef TILESETICONMANAGER_H
#define TILESETICONMANAGER_H

#include <QIconEngine>

namespace GfxPaint {

class TileSetIconManager;

class TileSetIconEngine : public QIconEngine
{
public:
protected:
    TileSetIconManager &manager;
    uint index;

    // QIconEngine interface
public:
    virtual void paint(QPainter *painter, const QRect &rect, QIcon::Mode mode, QIcon::State state) override;
    virtual QIconEngine *clone() const override;
};

class TileSetIconManager : public QObject
{
    Q_OBJECT
public:
    TileSetIconManager(QImage *tileSet, const QSize iconSize = {16, 16}, QObject *const parent = nullptr);
    QIconEngine *iconEngine(const uint index);
protected:
    QImage *tileSet;
};

} // namespace GfxPaint

#endif // TILESETICONMANAGER_H
