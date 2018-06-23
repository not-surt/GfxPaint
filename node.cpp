#include "node.h"

#include <QMetaObject>
#include <QMetaProperty>

#include "scene.h"
#include "application.h"
#include "utils.h"
#include "newbufferdialog.h"

namespace GfxPaint {

void Node::beforeChildren(Traversal &traversal)
{
//    // Type properties
//    for (const QMetaObject *meta = metaObject(); meta; meta = meta->superClass()) {
//        qDebug() << meta->className();
//        for (int i = 0; i < metaObject()->propertyCount(); ++i) {
//            QMetaProperty metaProperty = metaObject()->property(i);
//            qDebug() << metaProperty.name() << property(metaProperty.name());
//        }
//        meta = meta->superClass();
//    }
//    // Dynamic properties
//    for (auto name : dynamicPropertyNames()) {
//        qDebug() << name << property(name);
//    }
}

void Node::afterChildren(Traversal &traversal) {}

void SpatialNode::beforeChildren(Traversal &traversal)
{
    Node::beforeChildren(traversal);

    traversal.transformStack.push(traversal.transformStack.top() * combinedTransform());
}

void SpatialNode::afterChildren(Traversal &traversal)
{
    Node::afterChildren(traversal);

    traversal.transformStack.pop();
}

QMatrix4x4 SpatialNode::transform() const
{
    return m_transform;
}

void SpatialNode::setTransform(const QMatrix4x4 &transform)
{
    m_transform = transform;
}

QMatrix4x4 SpatialNode::combinedTransform() const
{
    return transform();
}

AbstractBufferNode::AbstractBufferNode(const Buffer &buffer, const bool indexed) :
    buffer(buffer), indexed(indexed)
{
}

AbstractBufferNode::AbstractBufferNode(const AbstractBufferNode &other) :
    buffer(other.buffer), indexed(other.indexed)
{
}

BufferNode::BufferNode(const Buffer &buffer, const bool indexed, const int blendMode, const int composeMode, const Colour transparent, const QSizeF &pixelAspectRatio, const QSizeF &scrollScale) :
    SpatialNode(), AbstractBufferNode(buffer, indexed),
    blendMode(blendMode), composeMode(composeMode), transparent(transparent),
    pixelAspectRatio(pixelAspectRatio), scrollScale(scrollScale)
{
}

BufferNode::BufferNode(const BufferNode &other) :
    SpatialNode(other), AbstractBufferNode(other),
    blendMode(other.blendMode), composeMode(other.composeMode), transparent(other.transparent),
    scrollScale(other.scrollScale)
{
}

Node *BufferNode::createFromFile(const QString &filename)
{
    ContextBinder binder(&qApp->renderManager.context, &qApp->renderManager.surface);
    Buffer palette;
    Colour transparent;
    Buffer buffer = bufferFromImageFile(filename, &palette, &transparent);
    if (!buffer.isNull()) {
        BufferNode *bufferNode = new BufferNode(buffer, !palette.isNull(), 0, RenderManager::composeModeDefault, transparent);
        if (!palette.isNull()) {
            PaletteNode *paletteNode = new PaletteNode(palette, false);
            paletteNode->insertChild(paletteNode->children.size(), bufferNode);
            return paletteNode;
        }
        else return bufferNode;
    }
    else return nullptr;
}

Node *BufferNode::createFromDialog(QWidget *const parentWindow)
{
    NewBufferDialog dialog(parentWindow);
    const int result = dialog.exec();
    if (result == QDialog::Accepted) {
        ContextBinder binder(&qApp->renderManager.context, &qApp->renderManager.surface);
        Buffer buffer(dialog.imageSize(), dialog.format());
        return new BufferNode(buffer, dialog.indexed(), 0, RenderManager::composeModeDefault, Colour{}, dialog.pixelRatio());
    }
    else return nullptr;
}

BufferNode *BufferNode::clone() const
{
    ContextBinder binder(&qApp->renderManager.context, &qApp->renderManager.surface);
    return new BufferNode(*this);
}

QMatrix4x4 BufferNode::combinedTransform() const
{
    return this->transform() * pixelAspectRatioTransform();
}

void BufferNode::beforeChildren(Traversal &traversal)
{
    SpatialNode::beforeChildren(traversal);

    render(traversal);
}

void BufferNode::afterChildren(Traversal &traversal)
{
    SpatialNode::afterChildren(traversal);
}

void BufferNode::render(Traversal &traversal)
{
    if (!traversal.renderTargetStack.isEmpty()) {
        const Traversal::RenderTarget &renderTarget = traversal.renderTargetStack.top();
        QMatrix4x4 transform = traversal.transformStack.top();
        const Buffer *palette = nullptr;
        Buffer::Format paletteFormat;
        if (!traversal.paletteStack.isEmpty()) {
            palette = traversal.paletteStack.top();
            paletteFormat = palette->format();
        }
        // TODO: don't create new program for every render
        BufferProgram program(buffer.format(), indexed, paletteFormat, renderTarget.buffer->format(), renderTarget.indexed, renderTarget.palette ? renderTarget.palette->format() : Buffer::Format(), 0, 3);
        renderTarget.buffer->bindFramebuffer();
        QMatrix4x4 workMatrix;
        workMatrix.scale(buffer.width(), buffer.height());
        program.render(&buffer, palette, transparent, renderTarget.transform * transform * workMatrix, renderTarget.buffer, renderTarget.palette);
    }
}

PaletteNode::PaletteNode(const Buffer &buffer, const bool indexed) :
    Node(), AbstractBufferNode(buffer, indexed)
{
}

PaletteNode::PaletteNode(const PaletteNode &other) :
    Node(other), AbstractBufferNode(other)
{
}

Node *PaletteNode::create()
{
    ContextBinder binder(&qApp->renderManager.context, &qApp->renderManager.surface);
    return new PaletteNode(Buffer({256, 1}, Buffer::Format(Buffer::Format::ComponentType::UInt, 1, 4)), false);
}

Node *PaletteNode::createFromFile(const QString &filename)
{
    ContextBinder binder(&qApp->renderManager.context, &qApp->renderManager.surface);
    Buffer palette;
    Buffer buffer = bufferFromImageFile(filename, &palette);
    if (!palette.isNull()) {
        return new PaletteNode(palette, false);
    }
    else if (!buffer.isNull()) {
        return new PaletteNode(palette, false);
    }
    else return nullptr;
}

PaletteNode *PaletteNode::clone() const
{
    ContextBinder binder(&qApp->renderManager.context, &qApp->renderManager.surface);
    return new PaletteNode(*this);
}

void PaletteNode::beforeChildren(Traversal &traversal)
{
    Node::beforeChildren(traversal);

    traversal.paletteStack.push(&buffer);
}

void PaletteNode::afterChildren(Traversal &traversal)
{
    Node::afterChildren(traversal);

    traversal.paletteStack.pop();
}

SamplerNode::SamplerNode(const Buffer &buffer, const bool indexed) :
    SpatialNode(), AbstractBufferNode(buffer, indexed)
{
}

SamplerNode::SamplerNode(const SamplerNode &other) :
    SpatialNode(other), AbstractBufferNode(other)
{
}

} // namespace GfxPaint
