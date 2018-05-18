#include "transformmodel.h"

#include <QtDebug>

namespace GfxPaint {

const TransformModel::Getter TransformModel::getters[3][3] = {
    {&QTransform::m11, &QTransform::m12, &QTransform::m13},
    {&QTransform::m21, &QTransform::m22, &QTransform::m23},
    {&QTransform::m31, &QTransform::m32, &QTransform::m33},
};

TransformModel::TransformModel(QObject *parent) :
    QAbstractTableModel(parent),
    m_transform()
{
}

int TransformModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    else return 3;
}

int TransformModel::columnCount(const QModelIndex &parent) const
{
    if (parent.isValid()) return 0;
    else return 3;
}

QVariant TransformModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid()) return QVariant();
    else if (role == Qt::DisplayRole || role == Qt::EditRole) {
        const qreal value = (m_transform.*getters[index.row()][index.column()])();
        if (role == Qt::DisplayRole) return QString::number(value, 'f', 3);
        else if (role == Qt::EditRole) return value;
    }
    else return QVariant();
}

bool TransformModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (role == Qt::EditRole && data(index, role) != value) {
        qreal values[3][3];
        for (int row = 0; row < 3; ++row) {
            for (int col = 0; col < 3; ++col) {
                values[row][col] = (m_transform.*getters[row][col])();
            }
        }
        values[index.row()][index.column()] = value.toDouble();
        m_transform.setMatrix(
            values[0][0], values[0][1], values[0][2],
            values[1][0], values[1][1], values[1][2],
            values[2][0], values[2][1], values[2][2]);
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

const QTransform &TransformModel::transform() const
{
    return m_transform;
}

void TransformModel::setTransform(const QTransform &transform)
{
    if (this->m_transform != transform) {
        beginResetModel();
        this->m_transform = transform;
        endResetModel();
        emit transformChanged(this->m_transform);
    }
}

} // namespace GfxPaint
