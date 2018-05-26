#include "editingcontext.h"

#include "application.h"

namespace GfxPaint {

EditingContext::EditingContext(Scene &scene) :
    scene(scene),
    m_brush(),
    m_colour(255, 0, 0, 255),
    m_dabPrograms(),
    m_colourPickPrograms(),
    m_workBuffers(),
    m_selectionModel(qApp->documentManager.documentModel(&scene))
{
}

EditingContext::EditingContext(EditingContext &other) :
    scene(other.scene),
    m_brush(other.m_brush),
    m_colour(other.m_colour),
    m_dabPrograms(other.m_dabPrograms),
    m_colourPickPrograms(other.m_colourPickPrograms),
    m_workBuffers(other.m_workBuffers),
    m_selectionModel(other.m_selectionModel.model())
{
    m_selectionModel.select(other.m_selectionModel.selection(), QItemSelectionModel::ClearAndSelect);
}

EditingContext::~EditingContext()
{
    ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
    qDeleteAll(m_dabPrograms);
    qDeleteAll(m_colourPickPrograms);
    qDeleteAll(m_workBuffers);
}

void EditingContext::updatePrograms()
{
    ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
    QMap<Node *, DabProgram *> oldDabPrograms = m_dabPrograms;
    m_dabPrograms.clear();
    QMap<Node *, ColourPickProgram *> oldColourPickPrograms = m_colourPickPrograms;
    m_colourPickPrograms.clear();
    Buffer buffer;
    scene.render(nullptr, false, nullptr, QTransform(), &m_states);
    for (auto index : m_selectionModel.selectedRows()) {
        Node *node = static_cast<Node *>(index.internalPointer());
        BufferNode *const bufferNode = dynamic_cast<BufferNode *>(node);
        if (bufferNode) {
            Traversal::State &state = m_states[node];
            if (bufferNode) {
                m_dabPrograms.insert(bufferNode, new DabProgram(m_brush.dab.type, m_brush.dab.metric, bufferNode->buffer.format(), bufferNode->indexed, state.palette ? state.palette->format() : Buffer::Format(), m_brush.dab.blendMode, m_brush.dab.composeMode));
                m_colourPickPrograms.insert(bufferNode, new ColourPickProgram(bufferNode->buffer.format()));
            }
        }
    }
    qDeleteAll(oldDabPrograms);
    qDeleteAll(oldColourPickPrograms);
}

void EditingContext::updateWorkBuffers()
{
    ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
    QMap<Node *, Buffer *> oldWorkBuffers = m_workBuffers;
    m_workBuffers.clear();
    for (auto index : m_selectionModel.selectedRows()) {
        Node *node = static_cast<Node *>(index.internalPointer());
        BufferNode *const bufferNode = dynamic_cast<BufferNode *>(node);
        if (bufferNode) {
            m_workBuffers.insert(bufferNode, new Buffer(bufferNode->buffer));
        }
    }
    qDeleteAll(oldWorkBuffers);
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

} // namespace GfxPaint
