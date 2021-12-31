#include "editingcontext.h"

#include "application.h"

namespace GfxPaint {

EditingContext::NodeEditingContext::NodeEditingContext(EditingContext *const context, Node *const node, Tool *const tool, PixelLineProgram * const pixelLineProgram, BrushDabProgram * const brushDabProgram, ColourPickProgram * const colourPickProgram, Buffer * const restoreBuffer)
    : pixelLineProgram(pixelLineProgram), brushDabProgram(brushDabProgram), colourPickProgram(colourPickProgram), restoreBuffer(restoreBuffer)/*,
    programs(tool->nodePrograms(*context, node))*/
{
}

EditingContext::NodeEditingContext::~NodeEditingContext() {
    delete pixelLineProgram;
    delete brushDabProgram;
    delete colourPickProgram;
    delete restoreBuffer;
}

const std::map<EditingContext::TransformTarget, QString> EditingContext::transformTargetNames{
    {TransformTarget::View, "View"},
    {TransformTarget::Object, "Object"},
    {TransformTarget::Brush, "Brush"},
};

const std::map<EditingContext::ToolSpace, QString> EditingContext::toolSpaceNames{
    {ToolSpace::Object, "Object Space"},
    {ToolSpace::ObjectAspectCorrected, "Aspect-corrected Object Space"},
    {ToolSpace::World, "World Space"},
    {ToolSpace::View, "View Space"},
};

EditingContext::EditingContext(Scene &scene) :
    scene(scene),
    m_toolSpace(ToolSpace::Object), m_blendMode{0}, m_composeMode{RenderManager::composeModeDefault},
    m_brush(),
    m_colour{{0.0, 0.0, 0.0, 1.0}, INDEX_INVALID},
    toolStroke{}, toolMode(0), transformTarget(TransformTarget::View),
    m_palette(nullptr),
    m_nodeEditingContexts(),
    m_selectionModel(qApp->documentManager.documentModel(&scene)),
    m_selectedNodes()
{
    update();
}

EditingContext::EditingContext(EditingContext &other) :
    scene(other.scene),
    m_toolSpace(other.m_toolSpace), m_blendMode(other.m_blendMode), m_composeMode(other.m_composeMode),
    m_brush(other.m_brush),
    m_colour(other.m_colour),
    toolStroke(other.toolStroke), toolMode(other.toolMode), transformTarget(other.transformTarget),
    m_palette(other.m_palette),
    m_nodeEditingContexts(),
    m_selectionModel(other.m_selectionModel.model()),
    m_selectedNodes(other.m_selectedNodes)
{
    m_selectionModel.select(other.m_selectionModel.selection(), QItemSelectionModel::ClearAndSelect);
}

EditingContext::~EditingContext()
{
    ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
    qDeleteAll(m_nodeEditingContexts);
}

void EditingContext::update()
{
    for (auto i = m_nodeEditingContexts.constBegin(); i != m_nodeEditingContexts.constEnd(); ++i) {
        auto const &node = i.key();
        auto const &nodeEditingContext = i.value();
    }
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
    QHash<Node *, NodeEditingContext *> oldNodeContexts = m_nodeEditingContexts;
    m_nodeEditingContexts.clear();
    for (Node *node : m_selectedNodes) {
        Traversal::State &state = m_states[node];
        Tool *const tool = nullptr;
        BufferNode *const bufferNode = dynamic_cast<BufferNode *>(node);
        if (bufferNode) {
            m_nodeEditingContexts.insert(bufferNode, new NodeEditingContext(this, node, tool,
                new PixelLineProgram(bufferNode->buffer.format(), bufferNode->indexed, state.palette ? state.palette->format() : Buffer::Format(), m_blendMode, m_composeMode),
                new BrushDabProgram(m_brush.dab.type, m_brush.dab.metric, bufferNode->buffer.format(), bufferNode->indexed, state.palette ? state.palette->format() : Buffer::Format(), m_blendMode, m_composeMode),
                new ColourPickProgram(bufferNode->buffer.format(), bufferNode->indexed, state.palette ? state.palette->format() : Buffer::Format()),
                new Buffer(bufferNode->buffer)
            ));
        }
    }
    qDeleteAll(oldNodeContexts);
}

} // namespace GfxPaint
