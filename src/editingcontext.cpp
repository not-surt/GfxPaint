#include "editingcontext.h"

#include "application.h"

namespace GfxPaint {

const std::map<EditingContext::Space, QString> EditingContext::spaceNames{
    {Space::Object, "Object Space"},
    {Space::ObjectAspectCorrected, "Aspect-corrected Object Space"},
    {Space::World, "World Space"},
    {Space::View, "View Space"},
};

EditingContext::EditingContext(Scene &scene) :
    scene(scene),
    m_brush(),
    m_colour{{0.0, 0.0, 0.0, 1.0}, INDEX_INVALID},
    m_palette(nullptr),
    m_bufferNodeContexts(),
    m_selectionModel(qApp->documentManager.documentModel(&scene)),
    m_selectedNodes()
{
    update();
}

EditingContext::EditingContext(EditingContext &other) :
    scene(other.scene),
    m_brush(other.m_brush),
    m_colour(other.m_colour),
    m_palette(other.m_palette),
    m_bufferNodeContexts(other.m_bufferNodeContexts),
    m_selectionModel(other.m_selectionModel.model()),
    m_selectedNodes(other.m_selectedNodes)
{
    m_selectionModel.select(other.m_selectionModel.selection(), QItemSelectionModel::ClearAndSelect);
}

EditingContext::~EditingContext()
{
    ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
    qDeleteAll(m_bufferNodeContexts);
}

void EditingContext::update()
{
    m_states.clear();
    m_selectedNodes.clear();
    for (auto index : m_selectionModel.selectedRows()) {
        Node *node = static_cast<Node *>(index.internalPointer());
        m_states.insert(node, Traversal::State());
        m_selectedNodes.append(node);
    }
    // Update node states (non render)
    scene.render(nullptr, false, nullptr, Mat4(), &m_states);

    ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
    QHash<Node *, BufferNodeContext *> oldNodeContexts = m_bufferNodeContexts;
    m_bufferNodeContexts.clear();
    for (Node *node : m_selectedNodes) {
        BufferNode *const bufferNode = dynamic_cast<BufferNode *>(node);
        if (bufferNode) {
            Traversal::State &state = m_states[node];
            const Buffer::Format strokeBufferFormat(Buffer::Format::ComponentType::Float, 4, 4);
            m_bufferNodeContexts.insert(bufferNode, new BufferNodeContext(
                new DabProgram(m_brush.dab.type, m_brush.dab.metric, bufferNode->buffer.format(), bufferNode->indexed, state.palette ? state.palette->format() : Buffer::Format(), m_brush.dab.blendMode, m_brush.dab.composeMode),
                new ColourPickProgram(bufferNode->buffer.format(), bufferNode->indexed, state.palette ? state.palette->format() : Buffer::Format()),
                new DabProgram(m_brush.dab.type, m_brush.dab.metric, strokeBufferFormat, false, Buffer::Format(), m_brush.dab.blendMode, m_brush.dab.composeMode),
                new Buffer(bufferNode->buffer),
                new Buffer(bufferNode->buffer.size(), strokeBufferFormat)
            ));
        }
    }
    qDeleteAll(oldNodeContexts);
}

} // namespace GfxPaint
