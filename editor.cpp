#include "editor.h"

#include <QMouseEvent>
#include <QtMath>
#include <cmath>
#include "application.h"
#include "utils.h"

namespace GfxPaint {

Editor::Editor(Scene &scene, QWidget *parent) :
    RenderedWidget(parent),
    scene(scene), model(*qApp->documentManager.documentModel(&scene)),
    strokeTool(*this), pickTool(*this), panTool(*this), rotoZoomTool(*this), zoomTool(*this), rotateTool(*this),
    m_editingContext(scene),
    cameraTransform(), m_transformMode(),
    inputState{}, cursorPos(), cursorOver{false}, wheelDelta{}, pressure{}, rotation{},
    toolSet{
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
    inputState{}, cursorPos(), cursorOver{false}, wheelDelta{}, pressure{}, rotation{},
    toolSet(other.toolSet), toolStack{}
{
    setMouseTracking(true);
    setFocusPolicy(Qt::WheelFocus);

    QObject::connect(&m_editingContext.selectionModel(), &QItemSelectionModel::selectionChanged, this, &Editor::updateContext);
}

Editor::~Editor()
{
}

bool Editor::eventFilter(QObject *const watched, QEvent *const event)
{
    if (!hasFocus() && (event->type() == QEvent::KeyPress || event->type() == QEvent::KeyRelease))  {
        const QKeyEvent *const keyEvent = static_cast<QKeyEvent *>(event);
        QKeyEvent *eventCopy = new QKeyEvent(keyEvent->type(), keyEvent->key(), keyEvent->modifiers(), keyEvent->text(), keyEvent->isAutoRepeat(), keyEvent->count());
        QApplication::postEvent(this, eventCopy);
    }

    return false;
}

bool Editor::event(QEvent *const event)
{
    // Handle input event
    const QKeyEvent *const keyEvent = static_cast<QKeyEvent *>(event);
    if ((event->type() == QEvent::KeyRelease && !keyEvent->isAutoRepeat()) ||
            (event->type() == QEvent::KeyPress && !keyEvent->isAutoRepeat()) ||
            event->type() == QEvent::MouseButtonRelease ||
            event->type() == QEvent::MouseButtonPress ||
            event->type() == QEvent::MouseMove ||
            event->type() == QEvent::Wheel ||
            event->type() == QEvent::TabletPress ||
            event->type() == QEvent::TabletRelease ||
            event->type() == QEvent::TabletMove)  {
        // Keep track of press keys/buttons & mouse pos
        const QMouseEvent *const mouseEvent = static_cast<QMouseEvent *>(event);
        const QWheelEvent *const wheelEvent = static_cast<QWheelEvent *>(event);
        const QTabletEvent *const tabletEvent = static_cast<QTabletEvent *>(event);
        inputState.wheelDirections = {{false, false, false, false}};
        wheelDelta = {};
        pressure = 0.0;
        rotation = 0.0;
        if (event->type() == QEvent::KeyPress && !keyEvent->isAutoRepeat()) inputState.keys.insert(static_cast<Qt::Key>(keyEvent->key()));
        else if (event->type() == QEvent::KeyRelease && !keyEvent->isAutoRepeat()) inputState.keys.remove(static_cast<Qt::Key>(keyEvent->key()));
        else if (event->type() == QEvent::MouseButtonPress) inputState.mouseButtons.insert(mouseEvent->button());
        else if (event->type() == QEvent::MouseButtonRelease) inputState.mouseButtons.remove(mouseEvent->button());
        else if (event->type() == QEvent::TabletPress) inputState.mouseButtons.insert(tabletEvent->button());
        else if (event->type() == QEvent::TabletRelease) inputState.mouseButtons.remove(tabletEvent->button());
        else if (event->type() == QEvent::Wheel) {
            inputState.wheelDirections = {{wheelEvent->angleDelta().x() < 0, wheelEvent->angleDelta().x() > 0, wheelEvent->angleDelta().y() < 0, wheelEvent->angleDelta().y() > 0}};
            static const qreal stepSize = 8 * 15;
            wheelDelta = wheelEvent->angleDelta() / stepSize;
        }
        if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonRelease || event->type() == QEvent::MouseMove) {
            cursorPos = mouseEvent->localPos();
            pressure = 1.0;
        }
        if (event->type() == QEvent::TabletPress || event->type() == QEvent::TabletRelease || event->type() == QEvent::TabletMove) {
            cursorPos = tabletEvent->posF();
            pressure = tabletEvent->pressure();
//            rotation = tabletEvent->rotation();
            const qreal tiltX = sin(qDegreesToRadians(static_cast<qreal>(tabletEvent->xTilt())));
            const qreal tiltY = sin(qDegreesToRadians(static_cast<qreal>(tabletEvent->yTilt())));
            rotation = std::atan2(tiltY, tiltX);
        }

        bool consume = false;

        // Remove non-matching tools
        QMutableVectorIterator<std::pair<InputState, Tool *>> iterator(toolStack);
        while (iterator.hasNext()) {
            iterator.next();
            const InputState &trigger = iterator.value().first;
            Tool *const tool = iterator.value().second;
            if (!trigger.test(inputState)) {
                tool->end(mouseToViewport(cursorPos), pressure, rotation);
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
                    oldTool->end(mouseToViewport(cursorPos), pressure, rotation);
                }
                Tool *const tool = toolSet[trigger];
                auto pair = std::make_pair(trigger, tool);
                if (!toolStack.contains(pair)) {
                    toolStack.push(pair);
                    tool->begin(mouseToViewport(cursorPos), pressure, rotation);
                }
                grabMouse();
                consume = true;
            }
        }

        // Handle mouse wheel
        if (event->type() == QEvent::Wheel) {
            if (!toolStack.isEmpty()) {
                Tool *const tool = toolStack.top().second;
                tool->wheel(mouseToViewport(cursorPos), wheelDelta);
            }
            consume = true;
        }
        // Update current tool
        else if (event->type() == QEvent::MouseMove ||
                 event->type() == QEvent::TabletMove) {
            if (!toolStack.isEmpty()) {
                Tool *const tool = toolStack.top().second;
                tool->update(mouseToViewport(cursorPos), pressure, rotation);
                qDebug() << pressure << rotation;/////////////////////
            }
            consume = true;
        }

        return consume;
    }

    if (event->type() == QEvent::Enter) {
        cursorOver = true;
        return true;
    }
    else if (event->type() == QEvent::Leave) {
        cursorOver = false;
        return true;
    }
    else if (event->type() == QEvent::FocusOut ||
        event->type() == QEvent::WindowDeactivate) {
        inputState = {};
        toolStack = {};
        releaseMouse();
        cursorOver = false;
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
            if (cursorOver) {
                ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
                bufferNode->buffer.bindFramebuffer();
                glDisable(GL_DEPTH_TEST);
                glDisable(GL_BLEND);

//                const QPointF point = mouseToWorld(mapFromGlobal(QCursor::pos())); // lower latency?
                const QPointF point = mouseToWorld(cursorPos);
                const QPointF snappedPoint = pixelSnap(point);
                drawDab(m_editingContext.brush().dab, m_editingContext.colour(), *bufferNode, snappedPoint);
                bufferNodeContext->strokeBuffer->bindFramebuffer();
                drawDab(m_editingContext.brush().dab, m_editingContext.colour(), *bufferNode, snappedPoint);
            }
        }
    }    

    // Draw scene
    widgetBuffer->bindFramebuffer();
    scene.render(widgetBuffer, false, nullptr, cameraTransform * viewportTransform, &m_editingContext.states());

    // Undraw on-canvas brush preview
    for (auto index : m_editingContext.selectionModel().selectedRows()) {
        Node *node = static_cast<Node *>(index.internalPointer());
        const EditingContext::BufferNodeContext *const bufferNodeContext = m_editingContext.bufferNodeContext(node);
        BufferNode *const bufferNode = dynamic_cast<BufferNode *>(node);
        if (bufferNode && bufferNodeContext->workBuffer) {
            if (cursorOver) {
                bufferNode->buffer.copy(*bufferNodeContext->workBuffer);
            }
        }
    }
}

qreal Editor::strokeSegmentDabs(const QPointF startPos, const qreal startPressure, const qreal startRotation, const QPointF endPos, const qreal endPressure, const qreal endRotation, const QSizeF spacing, const qreal offset, QList<StrokeTool::Point> &output) {
    auto ellipsePolar = [](const qreal a, const qreal b, const qreal theta){
        return (a * b) / std::sqrt(std::pow(a, 2.0) * std::pow(sin(theta), 2.0) + std::pow(b, 2.0) * std::pow(cos(theta), 2.0));
    };
    const qreal a = spacing.width() / 2.0;
    const qreal b = spacing.height() / 2.0;
    const qreal theta = std::atan2(endPos.y() - startPos.y(), endPos.x() - startPos.x());
    const qreal increment = ellipsePolar(a, b, theta + ((2.0 * M_PI) / 360.0) * -startRotation) * 2.0;

    qreal pos = offset * increment;
    if (pos == 0.0) {
        output.append({startPos, startPressure, startRotation});
        pos += increment;
    }
    const QPointF delta(endPos.x() - startPos.x(), endPos.y() - startPos.y());
    const qreal length = std::hypot(delta.x(), delta.y());
    const QPointF step(delta.x() / length, delta.y() / length);
    while (pos < length) {
        output.append({QPointF(startPos.x() + pos * step.x(), startPos.y() + pos * step.y()), startPressure, startRotation});
        pos += increment;
    }

    return (pos - length) / increment;
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
