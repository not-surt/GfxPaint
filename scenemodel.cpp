#include "scenemodel.h"

#include "scene.h"

namespace GfxPaint {

SceneModel::SceneModel(Scene &scene, QObject *const parent) :
    QAbstractItemModel(parent),
    scene(scene),
    rootIndex(createIndex(0, 0, &scene.root))
{
}

QModelIndex SceneModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!parent.isValid()) { // must be root
        if (column == 0 && row == 0) return rootIndex;
        else return QModelIndex();
    } else {
        Node *const parentNode = nodeFromIndex(parent);
        if (column == 0 && row >= 0 && row < parentNode->children.size()) return createIndex(row, column, parentNode->children[row]);
        else return QModelIndex();
    }
}

QModelIndex SceneModel::parent(const QModelIndex &child) const
{
    Node *const parentNode = nodeFromIndex(child)->parent;
    if (!parentNode) // child must be root
        return QModelIndex();
    else {
        Node *const grandparentNode = parentNode->parent;
        if (!grandparentNode) // parent must be root
            return createIndex(0, 0, parentNode);
        else {
            const int parentNodeIndex = grandparentNode->children.indexOf(parentNode);
            return createIndex(parentNodeIndex, 0, parentNode);
        }
    }
}

QVariant SceneModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    static const QStringList headings = {"Layer"};
    if (role == Qt::DisplayRole) {
        if (orientation == Qt::Horizontal && section >= 0 && section < headings.size()) return headings[section];
        else return section;
    } else return QVariant();
}

int SceneModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) // child must be root
        return 1;
    else {
        Node *const parentNode = nodeFromIndex(parent);
        return parentNode->children.length();
    }
}

int SceneModel::columnCount(const QModelIndex &parent) const
{
    return 1;
}

QVariant SceneModel::data(const QModelIndex &index, int role) const
{
    Node *const node = nodeFromIndex(index);
    switch (role) {
    case Qt::DisplayRole:
        if (index.column() == 0) {
            if (!node->name.isEmpty()) return node->name;
            else if (!node->parent) return "Root";
            else return node->label();
        }
        break;
    case Qt::FontRole:
        if (index.column() == 0 && node->name.isEmpty()) {
            QFont font;
            font.setItalic(true);
            return font;
        }
        break;
    case Qt::EditRole:
        if (index.column() == 0) return node->name;
        break;
    case Qt::CheckStateRole:
        if (index.parent().isValid()) return node->enabled ? Qt::Checked : Qt::Unchecked;
        break;
    }
    return QVariant();
}

bool SceneModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    Node *const node = nodeFromIndex(index);
    switch (role) {
    case Qt::EditRole:
        if (index.column() == 0 && node) {
            node->name = value.toString();
            emit dataChanged(index, index, {role});
            return true;
        }
        break;
    case Qt::CheckStateRole:
        if (index.column() == 0 && index.parent().isValid()) {
            node->enabled = (value == Qt::Checked ? true : false);
            emit dataChanged(index, index, {role});
            return true;
        }
        break;
    }
    return false;
}

Qt::ItemFlags SceneModel::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags = Qt::NoItemFlags;
    if (!index.isValid()) flags |= Qt:: ItemIsDropEnabled; // allow drop on empty space
    if (index.column() == 0) flags |= Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsDropEnabled;
    if (index.parent().isValid()) flags |= Qt::ItemIsDragEnabled | Qt::ItemIsUserCheckable;
    return flags;
}

Qt::DropActions SceneModel::supportedDropActions() const
{
    return Qt::CopyAction | Qt::MoveAction/* | Qt::LinkAction*/;
}

QMimeData *SceneModel::mimeData(const QModelIndexList &indices) const
{
    MimeData *data = new MimeData();
    data->indices = indices;
    return data;
}

bool SceneModel::canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const
{
    if (!parent.isValid()) return true; // allow drop on empty space
    else if (column == 0 || column == -1) return true;
    else return false;
}

bool SceneModel::dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent)
{
    Q_ASSERT(row <= rowCount(parent));

    const QModelIndex &destParent = parent.isValid() ? parent : rootIndex; // allow drop on empty space
    const MimeData *const mimeData = static_cast<const MimeData *>(data);
    if (row < 0) row = rowCount(destParent);
    switch (action) {
    case Qt::MoveAction: {
        moveIndices(mimeData->indices, row, destParent);
        return true;
    }
    case Qt::CopyAction: {
        copyIndices(mimeData->indices, row, destParent);
        return true;
    }
    case Qt::LinkAction:
    default: {
        return false;
    }
    }
}

void SceneModel::moveIndices(const QModelIndexList &indices, int row, QModelIndex parent)
{
    Q_ASSERT(parent.isValid());
    Q_ASSERT(row >= 0 && row <= rowCount(parent));

    Node *const destParentNode = nodeFromIndex(parent);
    for (int i = 0; i < indices.size(); ++i) {
        const QModelIndex &index = indices[i];
        if (beginMoveRows(index.parent(), index.row(), index.row(), parent, row)) {
            Node *const sourceParentNode = nodeFromIndex(index.parent());
            Node *const node = sourceParentNode->removeChild(index.row());
            destParentNode->insertChild(row + i, node);
            endMoveRows();
        }
    }
    scene.setModified();
}

void SceneModel::copyIndices(const QModelIndexList &indices, int row, QModelIndex parent)
{
    Q_ASSERT(parent.isValid());
    Q_ASSERT(row >= 0 && row <= rowCount(parent));

    Node *const destParentNode = nodeFromIndex(parent);
    beginInsertRows(parent, row, row + indices.size() - 1);
    for (int i = 0; i < indices.size(); ++i) {
        Node *const sourceNode = nodeFromIndex(indices[i]);
        destParentNode->insertChild(row + i, sourceNode->cloneWithSubGraph());
    }
    endInsertRows();
    scene.setModified();
}

QModelIndexList SceneModel::insertNodes(const QList<Node *> &nodes, int row, QModelIndex parent)
{
    Q_ASSERT(parent.isValid());
    Q_ASSERT(row >= 0 && row <= rowCount(parent));

    QModelIndexList indices;
    Node *const destParentNode = nodeFromIndex(parent);
    beginInsertRows(parent, row, row + nodes.size() - 1);
    for (int i = 0; i < nodes.size(); ++i) {
        destParentNode->insertChild(row + i, nodes[i]);
        indices << createIndex(row + i, 0, nodes[i]);
    }
    endInsertRows();
    scene.setModified();
    return indices;
}

void SceneModel::eraseIndices(const QModelIndexList &indices)
{
    for (auto index : indices) {
        Node *const parentNode = nodeFromIndex(index.parent());
        beginRemoveRows(index.parent(), index.row(), index.row());
        parentNode->eraseChild(index.row());
        endRemoveRows();
    }
    scene.setModified();
}

} // namespace GfxPaint
