#ifndef TRANSFORMMODEL_H
#define TRANSFORMMODEL_H

#include <QAbstractTableModel>

#include <QTransform>

namespace GfxPaint {

class TransformModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    typedef qreal (QTransform::*Getter)() const;

    explicit TransformModel(QObject *parent = nullptr);

    int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    const QTransform &transform() const;

signals:
    void transformChanged(const QTransform &transform);

public slots:
    void setTransform(const QTransform &transform);

protected:
    static const Getter getters[3][3];

    QTransform m_transform;
};

} // namespace GfxPaint

#endif // TRANSFORMMODEL_H
