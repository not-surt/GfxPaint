#include "scene.h"

#include <QImageReader>
#include <QJsonDocument>
#include <QMdiSubWindow>
#include <functional>

#include "application.h"
#include "utils.h"

namespace GfxPaint {

Traversal::State::~State() {
    ContextBinder binder(&qApp->renderManager.context, &qApp->renderManager.surface);
    // TODO: crash with delete here
//    delete palette;
}

Scene::Scene(const QString &filename) :
    root(),
    bufferEditors(),
    file(),
    m_filename(filename),
    m_modified(false)
{}

Scene::Scene(const Scene &other) :
    root(other.root),
    bufferEditors(other.bufferEditors),
    m_filename(other.m_filename),
    m_modified(other.m_modified)
{}

Scene *Scene::clone() const
{
    return new Scene(*this);
}

const QString &Scene::typeId() const
{
    static const QString id = "Scene";
    return id;
}

QList<QByteArray> Scene::formats()
{
    QList<QByteArray> list = QImageReader::supportedImageFormats();
    list.append("scene.gfx");
    return list;
}

bool Scene::saveAs(const QString &filename)
{
    bool result = false;
    if (result) {
        m_modified = false;
        m_filename = filename;
    }
    return result;
}

bool Scene::save()
{
    return saveAs(m_filename);
}

QString Scene::filename() const
{
    return m_filename;
}

void Scene::setFilename(const QString &filename)
{
    m_filename = filename;
}

bool Scene::modified() const
{
    return m_modified;
}

void Scene::setModified(const bool modified)
{
    m_modified = modified;
}

Scene *Scene::open(const QString &filename)
{
    Buffer palette;
    Node *node = BufferNode::createFromFile(filename);
    if (node) {
        Scene *const scene = new Scene();
        scene->m_filename = filename;
        scene->root.insertChild(scene->root.children.length(), node);
        scene->m_modified = false;
        return scene;
    }
    return nullptr;
}

QJsonObject Scene::toJsonObject() const
{
    QJsonObject json;
    return json;
}

Scene *Scene::fromJsonObject(const QJsonObject &jsonObject)
{
    Scene *const scene = new Scene();
    return scene;
}

bool Scene::save(const QString &filename)
{
//    QString saveFilename = !filename.isEmpty() ? filename : this->filename;
//    if (saveJsonDocument(QJsonDocument(toJsonObject()), filename)) {
//        this->filename = saveFilename;
//        isDirty = false;
//        return true;
//    }
    return false;
}

void Scene::beforeChildren(Node *const node, Traversal &traversal)
{
    node->beforeChildren(traversal);

    if (traversal.saveStates && traversal.saveStates->contains(node)) {
        (*traversal.saveStates)[node] = traversal.state();
    }
}

void Scene::afterChildren(Node *const node, Traversal &traversal)
{
    node->afterChildren(traversal);
}

void Scene::renderSubGraph(Node *const node, Buffer *const buffer, const bool indexed, const Buffer *const palette, const Mat4 &viewTransform, const Mat4 &parentTransform, std::unordered_map<Node *, Traversal::State> *const saveStates)
{
    Traversal traversal;

    traversal.saveStates = saveStates;

    if (buffer) traversal.renderTargetStack.push({buffer, indexed, palette, viewTransform});
//    else traversal.renderTargetStack.push({});
    traversal.transformStack.push(parentTransform);
    if (palette) traversal.paletteStack.push(palette);

    traverse<Traversal &>(node, Scene::beforeChildren, Scene::afterChildren, traversal);
//    traverse(node, traversal);

    if (palette) traversal.paletteStack.pop();
    traversal.transformStack.pop();
    if (buffer) traversal.renderTargetStack.pop();
}

void Scene::render(Buffer *const buffer, const bool indexed, const Buffer *const palette, const Mat4 &viewTransform, std::unordered_map<Node *, Traversal::State> *const saveStates)
{
    renderSubGraph(&root, buffer, indexed, palette, viewTransform, Mat4(), saveStates);
}

} // namespace GfxPaint
