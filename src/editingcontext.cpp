#include "editingcontext.h"

#include "application.h"
#include "editor.h"

namespace GfxPaint {

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
    selectedToolId(ToolId::Pixel),
    toolMode(0), toolSpace(ToolSpace::Object), toolStroke{},
    transformTarget(TransformTarget::View),
    blendMode{0}, composeMode{RenderManager::composeModeDefault},
    brush(),
    colour{{0.0, 0.0, 0.0, 1.0}, INDEX_INVALID},
    palette(nullptr),
    m_selectionModel(qApp->documentManager.documentModel(&scene)),
    m_selectedNodes()
{
}

EditingContext::EditingContext(EditingContext &other) :
    scene(other.scene),
    selectedToolId(other.selectedToolId),
    toolMode(other.toolMode), toolSpace(other.toolSpace), toolStroke(other.toolStroke),
    transformTarget(other.transformTarget),
    blendMode(other.blendMode), composeMode(other.composeMode),
    brush(other.brush),
    colour(other.colour),
    palette(other.palette),
    m_selectionModel(other.m_selectionModel.model()),
    m_selectedNodes(other.m_selectedNodes),
    formatToolPrograms{}
{
    m_selectionModel.select(other.m_selectionModel.selection(), QItemSelectionModel::ClearAndSelect);
}

EditingContext::~EditingContext()
{
    ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
    selectedNodeRestoreBuffers.clear();
    for (auto &[key, programs] : formatToolPrograms) {
        programs.clear();
    }
    formatToolPrograms.clear();
}

void EditingContext::update(Editor &editor)
{
    qDebug() << "EditingContext::update";/////////////////////////
    m_states.clear();
    for (const auto &node : m_selectedNodes) {
        BufferNode *const bufferNode = dynamic_cast<BufferNode *>(node);
        if (bufferNode) {
            scene.bufferRemoveEditor(&bufferNode->buffer, &editor);
        }
    }
    m_selectedNodes.clear();
    for (const auto &index : m_selectionModel.selectedRows()) {
        Node *node = static_cast<Node *>(index.internalPointer());
        m_states[node] = Traversal::State();
        m_selectedNodes.push_back(node);
        BufferNode *const bufferNode = dynamic_cast<BufferNode *>(node);
        if (bufferNode) {
            scene.bufferAddEditor(&bufferNode->buffer, &editor);
        }
    }
    // Update node states (non render)
    // TODO: reuse states from actual render
    scene.render(nullptr, false, nullptr, Mat4(), &m_states);

    ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
    auto oldSelectedNodeRestoreBuffers = selectedNodeRestoreBuffers;
    selectedNodeRestoreBuffers.clear();
    auto oldFormatToolPrograms = formatToolPrograms;
    formatToolPrograms.clear();
    auto tools = editor.activeToolIds();
    qDebug() << "TOOLS:" << tools.size();
    for (Node *node : m_selectedNodes) {
        Traversal::State &state = m_states[node];
        BufferNode *const bufferNode = dynamic_cast<BufferNode *>(node);
        if (bufferNode) {
            selectedNodeRestoreBuffers[bufferNode] = new Buffer(bufferNode->buffer);
            for (const ToolId toolId : tools) {
                Tool *const tool = editor.toolInfo.at(toolId).tool;
                auto programs = tool->formatPrograms(*this, bufferNode->buffer.format(), bufferNode->indexed, state.palette ? state.palette->format() : Buffer::Format());
                qDebug() << "PROGRAMS!!!";//////////////////////////////////////////
                const std::tuple key{bufferNode->buffer.format(), bufferNode->indexed, state.palette ? state.palette->format() : Buffer::Format(), tool};
                if (!formatToolPrograms.contains(key)) {
                    formatToolPrograms[key] = programs;
                }
            }
        }
    }
    oldSelectedNodeRestoreBuffers.clear();
    oldFormatToolPrograms.clear();
}

Program *EditingContext::toolProgram(const Buffer::Format &bufferFormat, const bool indexed, const Buffer::Format &paletteFormat, Tool *const tool, const QString &name)
{
    auto key = std::tuple{bufferFormat, indexed, paletteFormat, tool};
    if (!formatToolPrograms.contains(key)) {
        formatToolPrograms[key] = tool->formatPrograms(*this, bufferFormat, indexed, paletteFormat);
    }
    Q_ASSERT(formatToolPrograms[key].contains(name));
    return formatToolPrograms[key][name];
}

} // namespace GfxPaint
