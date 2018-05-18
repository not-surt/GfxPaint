#ifndef SCENEMODEL_H
#define SCENEMODEL_H

#include <QAbstractItemModel>

#include <QMimeData>

#include "node.h"

namespace GfxPaint {

class Scene;

class SceneModel : public QAbstractItemModel
{
    Q_OBJECT

private:
    struct MimeData : public QMimeData {
        QModelIndexList indices;
    };

public:
    SceneModel(Scene &scene, QObject *const parent = nullptr);
    virtual ~SceneModel() {}

    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &child) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;
    int rowCount(const QModelIndex &parent) const override;
    int columnCount(const QModelIndex &parent) const override;
    QVariant data(const QModelIndex &index, int role) const override;
    bool setData(const QModelIndex &index, const QVariant &value, int role) override;
    Qt::ItemFlags flags(const QModelIndex &index) const override;
    Qt::DropActions supportedDropActions() const override;
    QMimeData *mimeData(const QModelIndexList &indices) const override;
    bool canDropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) const override;
    bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent) override;

    void moveIndices(const QModelIndexList &indices, int row, QModelIndex parent);
    void copyIndices(const QModelIndexList &indices, int row, QModelIndex parent);
    void insertNodes(const QList<Node *> &nodes, int row, QModelIndex parent);
    void eraseIndices(const QModelIndexList &indices);
    static Node *nodeFromIndex(const QModelIndex &index) { return index.isValid() ? static_cast<Node *>(index.internalPointer()) : nullptr; }
    QModelIndex indexFromNode(Node *const node) {
        const int index = node->parent ? node->parent->children.indexOf(node) : 0;
        return createIndex(index, 0, node);
    }

    Scene &scene;
    QModelIndex rootIndex;
};

} // namespace GfxPaint

#endif // SCENEMODEL_H
