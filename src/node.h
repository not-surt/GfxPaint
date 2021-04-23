#ifndef NODE_H
#define NODE_H

#include <QObject>
#include <QList>
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
#include "rendermanager.h"

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

    Mat4 transform() const;
    void setTransform(const Mat4 &transform);
    virtual Mat4 combinedTransform() const;

protected:
    Mat4 m_transform;
};

class AbstractBufferNode {
public:
    Buffer buffer;
    bool indexed;

    explicit AbstractBufferNode(const Buffer &buffer, const bool indexed);
    explicit AbstractBufferNode(const AbstractBufferNode &other);

protected:
    QString formatString() const {
        QString str;
        str += buffer.format().componentTypeName();
        str += ":" + QString::number(buffer.format().componentSize * 8) + "bpc";
        str += ":" + QString::number(buffer.format().componentCount);
        return str;
    }
};

class BufferNode : public SpatialNode, public AbstractBufferNode
{
public:
    int blendMode;
    int composeMode;
    Colour transparent;
    QSizeF pixelAspectRatio;
    QSizeF scrollScale;

    explicit BufferNode(const Buffer &buffer, const bool indexed, const int blendMode = 0, const int composeMode = RenderManager::composeModeDefault, const Colour &transparent = {RGBA_INVALID, INDEX_INVALID}, const QSizeF &pixelAspectRatio = {1.0, 1.0}, const QSizeF &scrollScale = {1.0, 1.0});
    explicit BufferNode(const BufferNode &other);

    static Node *createFromFile(const QString &filename);
    static Node *createFromDialog(QWidget *const parentWindow);
    virtual BufferNode *clone() const override;

    virtual QString typeName() const override { return "Buffer"; }
    virtual QString labelInfo() const override {
        QString info;
        info += QString("%1x%2:%3").arg(buffer.size().width()).arg(buffer.size().height()).arg(formatString());
        return info;
    }

    virtual Mat4 combinedTransform() const override;

    virtual void beforeChildren(Traversal &traversal) override;
    virtual void afterChildren(Traversal &traversal) override;

    Mat4 pixelAspectRatioTransform() const {
        Mat4 transform;
        transform.scale(pixelAspectRatio.width(), pixelAspectRatio.height());
        return transform;
    }

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
        info += QString("%1:%2").arg(buffer.size().width()).arg(formatString());
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
