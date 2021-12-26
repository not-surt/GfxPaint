#ifndef TRANSFORMMODEL_H
#define TRANSFORMMODEL_H

#include <QAbstractTableModel>

#include "types.h"

namespace GfxPaint {

class TransformModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    explicit TransformModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    virtual QVariant headerData(const int section, const Qt::Orientation orientation, const int role) const override;

    const Mat4 &transform() const;

signals:
    void transformChanged(const GfxPaint::Mat4 &transform);

public slots:
    void setTransform(const GfxPaint::Mat4 &transform);

protected:
    Mat4 m_transform;
};

} // namespace GfxPaint

#endif // TRANSFORMMODEL_H
