#ifndef NODE_H
#define NODE_H

#include <QObject>
#include <QList>
#include <QDebug>
#include <QOpenGLBuffer>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include <QOpenGLVertexArrayObject>
#include <QFileInfo>
#include <QJsonObject>

#include "types.h"
#include "buffer.h"
#include "opengl.h"
#include "program.h"

namespace GfxPaint {

class Scene;
class Traversal;

class Node : public QObject
{
    Q_OBJECT
    Q_PROPERTY(bool enabled MEMBER enabled)
    Q_PROPERTY(bool locked MEMBER locked)
public:
    QString name;
    bool enabled;
    bool locked;
    bool promoteChildren;
    Node *parent;
    QList<Node *> children;

    explicit Node() :
        QObject(),
        name(), enabled(true), locked(false), promoteChildren(false), parent(nullptr), children()
    {
        setProperty("objectName", name);
        setProperty("poop", "hello");
    }
    explicit Node(const Node &other) :
        QObject(),
        name(other.name), enabled(other.enabled), locked(other.locked), promoteChildren(other.promoteChildren), parent(nullptr), children()
    {}
    virtual ~Node() { for (auto child : children) delete child; }

    virtual QJsonObject toJsonObject() const {
        QJsonObject json;
        return json;
    }

    static Node *fromJsonObject(const QJsonObject &jsonObject) {
        Node *const node = new Node();
        return node;
    }

    static Node *create() { return new Node(); }
    virtual Node *clone() const { return new Node(*this); }
    virtual Node *cloneWithSubGraph() const {
        Node *node = clone();
        for (int i = 0; i < children.size(); ++i) {
            node->insertChild(i, children[i]->cloneWithSubGraph());
        }
        return node;
    }

    void insertChild(const int index, Node *const child) {
        children.insert(index, child);
        child->parent = this;
    }
    Node *replaceChild(const int index, Node *const newChild) {
        Node *const oldChild = children[index];
        children[index] = newChild;
        if (newChild) newChild->parent = this;
        return oldChild;
    }
    Node *removeChild(const int index) { return children.takeAt(index); }
    bool eraseChild(const int index) {
        Node *node = removeChild(index);
        if (node) delete node;
        return node != nullptr;
    }

    virtual bool isRenderTarget() { return false; }

    virtual QString typeName() const { return "Node"; }
    virtual QString labelInfo() const { return QString(); }
    virtual QString label() const {
        QString info = labelInfo();
        if (!info.isEmpty()) info.prepend(":");
        return typeName() + info;
    }

    virtual void beforeChildren(Traversal &traversal);
    virtual void afterChildren(Traversal &traversal);
};

class SpatialNode : public Node
{
public:
    explicit SpatialNode() :
        Node(),
        m_transform()
    {}
    explicit SpatialNode(const SpatialNode &other) :
        Node(other),
        m_transform(other.m_transform)
    {}

    static Node *create() { return new SpatialNode(); }
    virtual SpatialNode *clone() const override { return new SpatialNode(*this); }

    virtual QString typeName() const override { return "Spatial"; }

    virtual void beforeChildren(Traversal &traversal) override;
    virtual void afterChildren(Traversal &traversal) override;

    QTransform transform() const;
    void setTransform(const QTransform &transform);
    virtual QTransform combinedTransform() const;

protected:
    QTransform m_transform;
};

class AbstractBufferNode {
public:
    Buffer buffer;
    bool indexed;

    explicit AbstractBufferNode(const Buffer &buffer, const bool indexed);
    explicit AbstractBufferNode(const AbstractBufferNode &other);
};

class BufferNode : public SpatialNode, public AbstractBufferNode
{
public:
    int blender;
    QSizeF pixelAspectRatio;
    QSizeF scrollScale;

    explicit BufferNode(const Buffer &buffer, const bool indexed, const int blender = 0, const QSizeF &pixelAspectRatio = {1.0, 1.0}, const QSizeF &scrollScale = {1.0, 1.0});
    explicit BufferNode(const BufferNode &other);

    static Node *createFromFile(const QString &filename);
    static Node *createFromDialog(QWidget *const parentWindow);
    virtual BufferNode *clone() const override;

    virtual QString typeName() const override { return "Buffer"; }
    virtual QString labelInfo() const override {
        QString info;
        info += QString("%1x%2").arg(buffer.size().width()).arg(buffer.size().height());
        return info;
    }

    virtual QTransform combinedTransform() const override;

    virtual void beforeChildren(Traversal &traversal) override;
    virtual void afterChildren(Traversal &traversal) override;

    QTransform pixelAspectRatioTransform() const { return QTransform().scale(pixelAspectRatio.width(), pixelAspectRatio.height()); }

private:
    void render(Traversal &traversal);
};

class PaletteNode : public Node, public AbstractBufferNode {
public:
    explicit PaletteNode(const Buffer &palette, const bool indexed);
    explicit PaletteNode(const PaletteNode &other);

    static Node *create();
    static Node *createFromFile(const QString &filename);
    virtual PaletteNode *clone() const override;

    virtual QString typeName() const override { return "Palette"; }
    virtual QString labelInfo() const override {
        QString info;
        info += QString("%1").arg(buffer.size().width());
        return info;
    }

    virtual void beforeChildren(Traversal &traversal) override;
    virtual void afterChildren(Traversal &traversal) override;
};

class SamplerNode : public SpatialNode, public AbstractBufferNode {
public:
    explicit SamplerNode(const Buffer &buffer, const bool indexed);
    explicit SamplerNode(const SamplerNode &other);

    virtual QString typeName() const override { return "Sampler"; }
    virtual QString labelInfo() const override {
        QString info;
        info += QString("%1x%2").arg(buffer.size().width()).arg(buffer.size().height());
        return info;
    }
};

} // namespace GfxPaint

#endif // NODE_H
