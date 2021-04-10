#include "transformmodel.h"

#include <QtDebug>

namespace GfxPaint {

TransformModel::TransformModel(QObject *parent) :
    QAbstractTableModel(parent),
    m_transform()
{
}

int TransformModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    else return 4;
}

int TransformModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    else return 4;
}

QVariant TransformModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) return QVariant();
    else if (role == Qt::DisplayRole || role == Qt::EditRole) {
        const float value = m_transform(index.column(), index.row());
        if (role == Qt::DisplayRole) return QString::number(value, 'f', 3);
        else if (role == Qt::EditRole) return value;
    }
    return QVariant();
}

bool TransformModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::EditRole && data(index, role) != value) {
        m_transform(index.column(), index.row()) = value.toFloat();
        emit dataChanged(index, index, QVector<int>() << role);
        emit transformChanged(this->m_transform);
        return true;
    }
    else return false;
}

Qt::ItemFlags TransformModel::flags(const QModelIndex &index) const
{
    if (!index.isValid()) return Qt::NoItemFlags;
    else return Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsEnabled;
}

QVariant TransformModel::headerData(const int section, const Qt::Orientation orientation, const int role) const
{
    if (role == Qt::DisplayRole) return section;
    else return QVariant();
}

const QMatrix4x4 &TransformModel::transform() const
{
    return m_transform;
}

void TransformModel::setTransform(const QMatrix4x4 &transform)
{
    if (this->m_transform != transform) {
        beginResetModel();
        this->m_transform = transform;
        endResetModel();
        emit transformChanged(this->m_transform);
    }
}

} // namespace GfxPaint
