#include "documentsmodel.h"

#include <QDebug>
#include "scene.h"
#include "scenemodel.h"
#include "editor.h"

namespace GfxPaint {

DocumentsModel::DocumentsModel()
    : QAbstractListModel(),
      m_documents(), m_documentModels(), m_documentEditors()
{
}

DocumentsModel::~DocumentsModel()
{
    for (auto document : m_documents) {
        removeDocument(document);
        delete document;
    }
}

const QList<Scene *> &DocumentsModel::documents() const
{
    return m_documents;
}

int DocumentsModel::documentCount() const
{
    return m_documents.count();
}

QModelIndex DocumentsModel::documentModelIndex(Scene *const document)
{
    const int index = m_documents.indexOf(document);
    if (index >= 0) return createIndex(index, 0);
    else return QModelIndex();
}

void DocumentsModel::addDocument(Scene *const document)
{
    const int index = documentCount();
    beginInsertRows(QModelIndex(), index, index);
    m_documents.insert(index, document);
    m_documentModels.insert(document, new SceneModel(*document));
    m_documentEditors.insert(document, QList<Editor *>());
    endInsertRows();
}

void DocumentsModel::removeDocument(Scene *const document)
{
    Q_ASSERT(m_documents.contains(document));

    const int index = m_documents.indexOf(document);
    beginRemoveRows(QModelIndex(), index, index);
    for (auto editor : m_documentEditors[document]) {
        removeEditor(editor);
        delete editor;
    }
    m_documentEditors.take(document);
    m_documentModels.take(document)->deleteLater();
    m_documents.removeAt(index);
    endRemoveRows();
}

SceneModel *DocumentsModel::documentModel(Scene *const document) const
{
    Q_ASSERT(m_documentModels.contains(document));

    return m_documentModels.value(document);
}

QList<Editor *> DocumentsModel::documentEditors(Scene *const document) const
{
    Q_ASSERT(m_documentEditors.contains(document));

    return m_documentEditors[document];
}

int DocumentsModel::documentEditorCount(Scene *const document) const
{
    Q_ASSERT(m_documentEditors.contains(document));
    return m_documentEditors[document].count();
}

void DocumentsModel::addEditor(Editor *const editor)
{
    Q_ASSERT(m_documents.contains(&editor->scene));

    m_documentEditors[&editor->scene].append(editor);
    const int index = m_documents.indexOf(&editor->scene);
    const QModelIndex modelIndex = createIndex(index, 0);
    emit dataChanged(modelIndex, modelIndex);
}

void DocumentsModel::removeEditor(Editor *const editor)
{
    Q_ASSERT(m_documentEditors.contains(&editor->scene));

    const int index = m_documentEditors[&editor->scene].indexOf(editor);
    m_documentEditors[&editor->scene].removeAt(index);
    const QModelIndex modelIndex = createIndex(index, 0);
    emit dataChanged(modelIndex, modelIndex);
}

int DocumentsModel::rowCount(const QModelIndex &parent) const
{
    if (!parent.isValid()) return documentCount();
    else return 0;
}

Qt::ItemFlags DocumentsModel::flags(const QModelIndex &index) const
{
    return QAbstractListModel::flags(index) | Qt::ItemIsSelectable | Qt::ItemIsEditable | Qt::ItemIsUserCheckable | Qt::ItemNeverHasChildren;
}

QVariant DocumentsModel::data(const QModelIndex &index, int role) const
{
    if (!index.parent().isValid()) {
        Scene *const document = m_documents[index.row()];
        Q_ASSERT(document);
        switch (role) {
        case Qt::DisplayRole:
            return document->filename() + (document->modified() ? "*" : "") + " (" + QString::number(documentEditorCount(m_documents[index.row()])) + ")";
        case Qt::EditRole:
            return document->filename();
        case Qt::CheckStateRole:
            return Qt::Checked;
        }
    }
    return QVariant();
}

bool DocumentsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.parent().isValid()) {
        Scene *const document = m_documents[index.row()];
        switch (role) {
        case Qt::EditRole:
            document->setFilename(value.toString());
            emit dataChanged(index, index, {Qt::DisplayRole});
            return true;
        case Qt::CheckStateRole:
            emit closeDocument(document);
            return true;
        }
    }
    return false;
}

} // namespace GfxPaint
