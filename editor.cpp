#include "editor.h"

#include <QMouseEvent>
#include <QtMath>
#include "application.h"
#include "utils.h"

namespace GfxPaint {

//uint qHash(const std::tuple<bool, bool, bool, bool> &key, uint seed) {
//    return (static_cast<uint>(key.axis) + seed) << key.decrease << key.increase;
//}

Editor::Editor(Scene &scene, QWidget *parent) :
    RenderedWidget(parent),
    scene(scene), model(*qApp->documentManager.documentModel(&scene)),
    strokeTool(*this), pickTool(*this), panTool(*this), rotoZoomTool(*this), zoomTool(*this), rotateTool(*this),
    m_editingContext(scene),
    cameraTransform(), m_transformMode(),
    inputState{}, mousePos(), mouseOver{false}, wheelDelta{},
    toolSet{
//        {{{}, {}, {{false, false, true, true}}}, &strokeTool},
        {{{}, {Qt::LeftButton}, {}}, &strokeTool},
        {{{}, {Qt::MiddleButton}, {}}, &panTool},
        {{{Qt::Key_Space}, {}, {}}, &panTool},
        {{{}, {}, {{false, false, true, true}}}, &zoomTool},
        {{{Qt::Key_Control}, {Qt::LeftButton}, {}}, &pickTool},
        {{{Qt::Key_Control}, {Qt::MiddleButton}, {}}, &rotoZoomTool},
        {{{Qt::Key_Control, Qt::Key_Space}, {}, {}}, &rotoZoomTool},
        {{{Qt::Key_Control}, {}, {{false, false, true, true}}}, &rotateTool},
    },
    toolStack()
{
    setMouseTracking(true);
    setFocusPolicy(Qt::WheelFocus);

    QObject::connect(&m_editingContext.selectionModel(), &QItemSelectionModel::selectionChanged, this, &Editor::updateContext);
}

Editor::Editor(const Editor &other) :
    RenderedWidget(other.parentWidget()),
    scene(other.scene), model(other.model),
    strokeTool(other.strokeTool), pickTool(other.pickTool), panTool(other.panTool), rotoZoomTool(other.rotoZoomTool), zoomTool(other.zoomTool), rotateTool(other.rotateTool),
    m_editingContext(other.scene),
    cameraTransform(other.cameraTransform), m_transformMode(other.m_transformMode),
    inputState{}, mousePos(), mouseOver{false}, wheelDelta{},
    toolSet(other.toolSet), toolStack{}
{
    setMouseTracking(true);
    setFocusPolicy(Qt::WheelFocus);

    QObject::connect(&m_editingContext.selectionModel(), &QItemSelectionModel::selectionChanged, this, &Editor::updateContext);
}

Editor::~Editor()
{
}

bool Editor::event(QEvent *const event)
{
    // Keep track of press keys/buttons & mouse pos
    const QKeyEvent *const keyEvent = static_cast<QKeyEvent *>(event);
    const QMouseEvent *const mouseEvent = static_cast<QMouseEvent *>(event);
    const QWheelEvent *const wheelEvent = static_cast<QWheelEvent *>(event);
//    if ((event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease) && !keyEvent->isAutoRepeat()) return true;
    inputState.wheelDirections = {{false, false, false, false}};
    wheelDelta = {};
    if (event->type() == QEvent::KeyPress && !keyEvent->isAutoRepeat()) inputState.keys.insert(static_cast<Qt::Key>(keyEvent->key()));
    else if (event->type() == QEvent::KeyRelease && !keyEvent->isAutoRepeat()) inputState.keys.remove(static_cast<Qt::Key>(keyEvent->key()));
    else if (event->type() == QEvent::MouseButtonPress) inputState.mouseButtons.insert(mouseEvent->button());
    else if (event->type() == QEvent::MouseButtonRelease) inputState.mouseButtons.remove(mouseEvent->button());
    else if (event->type() == QEvent::Wheel) {
        inputState.wheelDirections = {{wheelEvent->angleDelta().x() < 0, wheelEvent->angleDelta().x() > 0, wheelEvent->angleDelta().y() < 0, wheelEvent->angleDelta().y() > 0}};
        static const qreal stepSize = 8 * 15;
        wheelDelta = wheelEvent->angleDelta() / stepSize;
    }
    if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonRelease || event->type() == QEvent::MouseMove) mousePos = mouseEvent->localPos();

    if ((event->type() == QEvent::KeyRelease && !keyEvent->isAutoRepeat()) ||
            event->type() == QEvent::MouseButtonRelease ||
            (event->type() == QEvent::KeyPress && !keyEvent->isAutoRepeat()) ||
            event->type() == QEvent::MouseButtonPress ||
            event->type() == QEvent::Wheel ||
            event->type() == QEvent::MouseMove)  {
        // Handle input event
        bool consume = false;
        // Remove non-matching tools
        QMutableVectorIterator<std::pair<InputState, Tool *>> iterator(toolStack);
        while (iterator.hasNext()) {
            iterator.next();
            const InputState &trigger = iterator.value().first;
            Tool *const tool = iterator.value().second;
            if (!trigger.test(inputState)) {
                tool->end(mouseToViewport(mousePos));
                iterator.remove();
                releaseMouse();
                consume = true;
            }
        }
        // Add matching tools
        for (auto trigger : toolSet.keys()) {
            if (trigger.test(inputState)) {
                if (!toolStack.isEmpty()) {
                    Tool *const oldTool = toolStack.top().second;
                    oldTool->end(mouseToViewport(mousePos));
                }
                Tool *const tool = toolSet[trigger];
                auto pair = std::make_pair(trigger, tool);
                if (!toolStack.contains(pair)) {
                    toolStack.push(pair);
                    tool->begin(mouseToViewport(mousePos));
                }
                grabMouse();
                consume = true;
            }
        }
        // Handle mouse wheel
        if (!toolStack.isEmpty() && !wheelDelta.isNull()) {
            Tool *const tool = toolStack.top().second;
            tool->wheel(mouseToViewport(mousePos), wheelDelta);
            consume = true;
        }
        if (event->type() == QEvent::MouseMove) {
            // Update current tool
            if (!toolStack.isEmpty()) {
                Tool *const tool = toolStack.top().second;
                tool->update(mouseToViewport(mousePos));
                consume = true;
            }
        }
        if (consume) return true;
    }
    else if (event->type() == QEvent::Enter) {
        mouseOver = true;
        return true;
    }
    else if (event->type() == QEvent::Leave) {
        mouseOver = false;
        return true;
    }
    else if (event->type() == QEvent::WindowDeactivate) {
        inputState = {};
        releaseMouse();
        mouseOver = false;
        return true;
    }

    return RenderedWidget::event(event);
}

void Editor::updateWindowTitle()
{
    setWindowModified(scene.modified());
    setWindowFilePath(scene.filename());
}

void Editor::setDocumentFilename(const QString &filename)
{
    scene.setFilename(filename);
    updateWindowTitle();
}

void Editor::setDocumentModified(const bool modified)
{
    scene.setModified(modified);
    updateWindowTitle();
}

QString Editor::label() const
{
    if (scene.filename().isNull()) return "unsaved";
    else return QFileInfo(scene.filename()).fileName();
}

void Editor::activate()
{
    emit brushChanged(m_editingContext.brush());
    emit transformModeChanged(this->m_transformMode);
    emit transformChanged(this->cameraTransform);
    updateContext();
}

void Editor::insertNodes(const QList<Node *> &nodes)
{
    const QModelIndexList rows = m_editingContext.selectionModel().selectedRows();
    if (rows.isEmpty() || rows.last().parent() == QModelIndex()) {
        model.insertNodes(nodes, scene.root.children.length(), model.rootIndex);
    }
    else {
        model.insertNodes(nodes, rows.last().row() + 1, rows.last().parent());
    }
}

void Editor::removeSelectedNodes()
{

}

void Editor::duplicateSelectedNodes()
{

}

void Editor::setBrush(const Brush &brush)
{
    if (m_editingContext.brush() != brush) {
        m_editingContext.setBrush(brush);
        emit brushChanged(m_editingContext.brush());
    }
}

void Editor::setColour(const Colour &colour)
{
    if (m_editingContext.colour() != colour) {
        m_editingContext.setColour(colour);
        emit colourChanged(m_editingContext.colour());
    }
}

void Editor::render()
{
    for (auto index : m_editingContext.selectionModel().selectedRows()) {
        Node *node = static_cast<Node *>(index.internalPointer());
        const EditingContext::BufferNodeContext *const bufferNodeContext = m_editingContext.bufferNodeContext(node);
        BufferNode *const bufferNode = dynamic_cast<BufferNode *>(node);
        if (bufferNode && bufferNodeContext->workBuffer) {
            bufferNodeContext->workBuffer->copy(bufferNode->buffer);
            // Draw on-canvas brush preview
            if (mouseOver) {
                ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
                bufferNode->buffer.bindFramebuffer();
                glDisable(GL_DEPTH_TEST);
                glDisable(GL_BLEND);

                const QPointF point = mouseToWorld(mapFromGlobal(QCursor::pos()));
//                const QPointF point = viewportToWorld(mousePos);
                const QPointF snappedPoint = pixelSnap(point);
                drawDab(m_editingContext.brush().dab, m_editingContext.colour(), *bufferNode, snappedPoint);
                bufferNodeContext->strokeBuffer->bindFramebuffer();
                drawDab(m_editingContext.brush().dab, m_editingContext.colour(), *bufferNode, snappedPoint);
            }
        }
    }    

    // Draw scene
    widgetBuffer->bindFramebuffer();
    scene.render(widgetBuffer, false, /*&palette*/nullptr, cameraTransform * viewportTransform, &m_editingContext.states());

    // Undraw on-canvas brush preview
    for (auto index : m_editingContext.selectionModel().selectedRows()) {
        Node *node = static_cast<Node *>(index.internalPointer());
        const EditingContext::BufferNodeContext *const bufferNodeContext = m_editingContext.bufferNodeContext(node);
        BufferNode *const bufferNode = dynamic_cast<BufferNode *>(node);
        if (bufferNode && bufferNodeContext->workBuffer) {
            if (mouseOver) {
                bufferNode->buffer.copy(*bufferNodeContext->workBuffer);
            }
        }
    }
}

qreal Editor::strokeSegmentDabs(const QPointF start, const QPointF end, const  qreal spacing, const qreal offset, QList<QPointF> &output) {
    const QSizeF _spacing(qMax(spacing, 1.0), qMax(spacing, 1.0));

    qreal pos = offset;
    if (pos == 0.0) {
        output.append(start);
        pos += _spacing.width();
    }
    const QPointF delta(end.x() - start.x(), end.y() - start.y());
    const qreal length = hypot(delta.x(), delta.y());
    const QPointF step(delta.x() / length, delta.y() / length);
    while (pos < length) {
        output.append(QPointF(start.x() + pos * step.x(), start.y() + pos * step.y()));
        pos += _spacing.width();
    }

    return pos - length;
}

void Editor::drawDab(const Dab &dab, const Colour &colour, BufferNode &node, const QPointF worldPos)
{
    Q_ASSERT( m_editingContext.bufferNodeContext(&node));

    const Traversal::State &state = m_editingContext.states().value(&node);

    const QTransform &transform = state.transform;
    QTransform spaceTransform;
    switch (dab.space) {
    case Space::Object: {
        spaceTransform = QTransform();
    } break;
    case Space::ObjectAspectCorrected: {
        spaceTransform = node.pixelAspectRatioTransform().inverted();
    } break;
    case Space::World: {
        spaceTransform = transform.inverted();
    } break;
    case Space::View: {
        spaceTransform = (transform * cameraTransform).inverted();
    } break;
    }
    const QPointF spaceOffset = spaceTransform.map(QPointF(0.0, 0.0));
    spaceTransform = spaceTransform * QTransform().translate(-spaceOffset.x(), -spaceOffset.y());
    const QPointF objectSpacePos = transform.inverted().map(worldPos);

    const Buffer *const palette = m_editingContext.states()[&node].palette;
    EditingContext::BufferNodeContext *const bufferNodeContext = m_editingContext.bufferNodeContext(&node);
    bufferNodeContext->dabProgram->render(dab, colour, spaceTransform * QTransform().translate(objectSpacePos.x(), objectSpacePos.y()) * GfxPaint::viewportTransform(node.buffer.size()), &node.buffer, palette);
}

void Editor::drawSegment(const Dab &dab, const Stroke &stroke, const Colour &colour, BufferNode &node, const QPointF start, const QPointF end, const qreal offset)
{
}

void Editor::setTransformMode(const TransformMode transformMode)
{
    if (this->m_transformMode != transformMode) {
        this->m_transformMode = transformMode;
        emit transformModeChanged(this->m_transformMode);
    }
}

void Editor::setTransform(const QTransform &transform)
{
    if (this->cameraTransform != transform) {
        this->cameraTransform = transform;
        emit transformChanged(this->cameraTransform);
        update();
    }
}

void Editor::updateContext()
{
    m_editingContext.states().clear();
    for (auto index : m_editingContext.selectionModel().selectedRows()) {
        Node *node = static_cast<Node *>(index.internalPointer());
        m_editingContext.states().insert(node, Traversal::State());
    }
    // Update node states (non render)
    scene.render(nullptr, false, nullptr, QTransform(), &m_editingContext.states());

    Buffer *palette = nullptr;
    for (auto index : m_editingContext.selectionModel().selectedRows()) {
        Node *node = static_cast<Node *>(index.internalPointer());
        const Traversal::State &state = m_editingContext.states().value(node);
        if (state.palette) palette = state.palette;
    }
    emit paletteChanged(palette);

    m_editingContext.updatePrograms();
}

} // namespace GfxPaint
