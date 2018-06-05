#include "editingcontext.h"

#include "application.h"

namespace GfxPaint {

EditingContext::EditingContext(Scene &scene) :
    scene(scene),
    m_brush(),
    m_colour(255, 0, 0, 255),
    m_palette(nullptr),
    m_bufferNodeContexts(),
    m_selectionModel(qApp->documentManager.documentModel(&scene))
{
}

EditingContext::EditingContext(EditingContext &other) :
    scene(other.scene),
    m_brush(other.m_brush),
    m_colour(other.m_colour),
    m_bufferNodeContexts(other.m_bufferNodeContexts),
    m_selectionModel(other.m_selectionModel.model())
{
    m_selectionModel.select(other.m_selectionModel.selection(), QItemSelectionModel::ClearAndSelect);
}

EditingContext::~EditingContext()
{
    ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
    qDeleteAll(m_bufferNodeContexts);
}

void EditingContext::updatePrograms()
{
    ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
    QMap<Node *, BufferNodeContext *> oldNodeContexts = m_bufferNodeContexts;
    m_bufferNodeContexts.clear();
    // Update node states (non render)
//    scene.render(nullptr, false, nullptr, QTransform(), &m_states);
    for (auto index : m_selectionModel.selectedRows()) {
        Node *node = static_cast<Node *>(index.internalPointer());
        BufferNode *const bufferNode = dynamic_cast<BufferNode *>(node);
        if (bufferNode) {
            Traversal::State &state = m_states[node];
            m_bufferNodeContexts.insert(bufferNode, new BufferNodeContext(
                new DabProgram(m_brush.dab.type, m_brush.dab.metric, bufferNode->buffer.format(), bufferNode->indexed, state.palette ? state.palette->format() : Buffer::Format(), m_brush.dab.blendMode, m_brush.dab.composeMode),
                new ColourPickProgram(bufferNode->buffer.format(), bufferNode->indexed, state.palette ? state.palette->format() : Buffer::Format()),
                new Buffer(bufferNode->buffer),
                new Buffer(bufferNode->buffer.size(), Buffer::Format(Buffer::Format::ComponentType::Float, 4, 3))
            ));
        }
    }
    qDeleteAll(oldNodeContexts);
}

void EditingContext::setBrush(const Brush &brush)
{
    this->m_brush = brush;
    updatePrograms();
}

void EditingContext::setColour(const QColor &colour)
{
    this->m_colour = colour;
}

void EditingContext::setPalette(Buffer *const palette)
{
    this->m_palette = palette;
}

} // namespace GfxPaint
