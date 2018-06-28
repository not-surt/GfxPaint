#include "editor.h"

#include <QMouseEvent>
#include <QtMath>
//#include <QGamepadManager>
#include <cmath>
#include "application.h"
#include "utils.h"

namespace GfxPaint {

void Editor::init()
{
    setMouseTracking(true);
    setFocusPolicy(Qt::WheelFocus);

    QObject::connect(&m_editingContext.selectionModel(), &QItemSelectionModel::selectionChanged, this, &Editor::updateContext);

//    qDebug() << QGamepadManager::instance()->isGamepadConnected(0);
//    QObject::connect(QGamepadManager::instance(), &QGamepadManager::gamepadAxisEvent, this, [](){

//    });
//    QObject::connect(QGamepadManager::instance(), &QGamepadManager::gamepadButtonPressEvent, this, [](){

//    });
//    QObject::connect(QGamepadManager::instance(), &QGamepadManager::gamepadButtonReleaseEvent, this, [](){

//    });
}

Editor::Editor(Scene &scene, QWidget *parent) :
    RenderedWidget(parent),
    scene(scene), model(*qApp->documentManager.documentModel(&scene)),
    strokeTool(*this), pickTool(*this), panTool(*this), rotoZoomTool(*this), zoomTool(*this), rotateTool(*this),
    m_editingContext(scene),
    cameraTransform(), m_transformMode(),
    inputState{}, cursorPos(), cursorDelta(), cursorOver{false}, wheelDelta{}, pressure{}, rotation{}, tilt{}, quaternion{},
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
    init();
}

Editor::Editor(const Editor &other) :
    RenderedWidget(other.parentWidget()),
    scene(other.scene), model(other.model),
    strokeTool(other.strokeTool), pickTool(other.pickTool), panTool(other.panTool), rotoZoomTool(other.rotoZoomTool), zoomTool(other.zoomTool), rotateTool(other.rotateTool),
    m_editingContext(other.scene),
    cameraTransform(other.cameraTransform), m_transformMode(other.m_transformMode),
    inputState{}, cursorPos(), cursorDelta(), cursorOver{false}, wheelDelta{}, pressure{}, rotation{}, tilt{}, quaternion{},
    toolSet(other.toolSet), toolStack{}
{
    init();
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
    else if (event->type() == QEvent::Wheel)  {
        const QWheelEvent *const wheelEvent = static_cast<QWheelEvent *>(event);
        qDebug() << "Window Wheel" << wheelEvent->angleDelta();//////////////////
//        QWheelEvent *eventCopy = new QWheelEvent(wheelEvent->pos(), wheelEvent->globalPos(), wheelEvent->pixelDelta(), wheelEvent->angleDelta(), wheelEvent->delta(), wheelEvent->orientation(), wheelEvent->buttons(), wheelEvent->modifiers(), wheelEvent->phase(), wheelEvent->source());
//        QApplication::postEvent(this, eventCopy);
//        return true;
    }
    else return RenderedWidget::eventFilter(watched, event);

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
        cursorDelta = {0.0f, 0.0f};
        wheelDelta = {0.0f, 0.0f};
        pressure = 1.0f;
        rotation = 0.0f;
        tilt = {0.0f, 0.0f};
        quaternion = QQuaternion(0.0f, 0.0f, 0.0f, 1.0f);
        if (event->type() == QEvent::KeyPress && !keyEvent->isAutoRepeat()) inputState.keys.insert(static_cast<Qt::Key>(keyEvent->key()));
        else if (event->type() == QEvent::KeyRelease && !keyEvent->isAutoRepeat()) inputState.keys.remove(static_cast<Qt::Key>(keyEvent->key()));
        else if (event->type() == QEvent::MouseButtonPress) inputState.mouseButtons.insert(mouseEvent->button());
        else if (event->type() == QEvent::MouseButtonRelease) inputState.mouseButtons.remove(mouseEvent->button());
        else if (event->type() == QEvent::TabletPress) inputState.mouseButtons.insert(tabletEvent->button());
        else if (event->type() == QEvent::TabletRelease) inputState.mouseButtons.remove(tabletEvent->button());
        else if (event->type() == QEvent::Wheel) {
            inputState.wheelDirections = {{wheelEvent->angleDelta().x() < 0, wheelEvent->angleDelta().x() > 0, wheelEvent->angleDelta().y() < 0, wheelEvent->angleDelta().y() > 0}};
            static const float stepSize = 8 * 15;
            wheelDelta = QVector2D(wheelEvent->angleDelta()) / stepSize;
        }
        if (event->type() == QEvent::MouseButtonRelease || event->type() == QEvent::MouseMove) {
            cursorDelta = QVector2D(mouseEvent->localPos()) - cursorPos;
        }
        if (event->type() == QEvent::TabletRelease || event->type() == QEvent::TabletMove) {
            cursorDelta = QVector2D(tabletEvent->posF()) - cursorPos;
        }
        if (event->type() == QEvent::MouseButtonPress || event->type() == QEvent::MouseButtonRelease || event->type() == QEvent::MouseMove) {
            cursorPos = QVector2D(mouseEvent->localPos());
            pressure = 1.0f;
        }
        if (event->type() == QEvent::TabletPress || event->type() == QEvent::TabletRelease || event->type() == QEvent::TabletMove) {
            cursorPos = QVector2D(tabletEvent->posF());
            pressure = static_cast<float>(tabletEvent->pressure());
            rotation = static_cast<float>(tabletEvent->rotation());
            tilt = {qDegreesToRadians(static_cast<float>(tabletEvent->xTilt())), qDegreesToRadians(static_cast<float>(tabletEvent->yTilt()))};
            rotation = qRadiansToDegrees(std::atan2(std::sin(tilt.y()), std::sin(tilt.x())) + (tau<float> / 4.0f));
            quaternion = QQuaternion::fromEulerAngles(tilt.y(), tilt.x(), rotation);
        }
        Point point{QVector2D(mouseToWorld(cursorPos)), static_cast<float>(pressure), quaternion};

        bool consume = false;

        // Remove non-matching tools
        QMutableVectorIterator<std::pair<InputState, Tool *>> iterator(toolStack);
        while (iterator.hasNext()) {
            iterator.next();
            const InputState &trigger = iterator.value().first;
            Tool *const tool = iterator.value().second;
            if (!trigger.test(inputState)) {
                tool->end(mouseToViewport(cursorPos), point);
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
                    oldTool->end(mouseToViewport(cursorPos), point);
                }
                Tool *const tool = toolSet[trigger];
                auto pair = std::make_pair(trigger, tool);
                if (!toolStack.contains(pair)) {
                    toolStack.push(pair);
                    tool->begin(mouseToViewport(cursorPos), point);
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
                tool->update(mouseToViewport(cursorPos), point);
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

                const QVector2D point = QVector2D(mouseToWorld(cursorPos));
                const QVector2D snappedPoint = pixelSnap(point);
//                drawDab(m_editingContext.brush().dab, m_editingContext.colour(), *bufferNode, snappedPoint);
                Tool *const tool = &strokeTool;
                tool->preview(mouseToViewport(cursorPos), {mouseToWorld(cursorPos), 1.0, quaternion});
//                bufferNodeContext->strokeBuffer->bindFramebuffer();
//                drawDab(m_editingContext.brush().dab, m_editingContext.colour(), *bufferNode, snappedPoint);
            }
        }
    }    

    // Draw scene
    widgetBuffer->bindFramebuffer();
    scene.render(widgetBuffer, false, nullptr, viewportTransform * cameraTransform, &m_editingContext.states());

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

float Editor::strokeSegmentDabs(const Stroke::Point &start, const Stroke::Point &end, const QVector2D dabSize, const QVector2D absoluteSpacing, const QVector2D proportionalSpacing, const float offset, Stroke &output) {
    auto ellipsePolar = [](const float a, const float b, const float theta){
        return (a * b) / std::sqrt(std::pow(a, 2.0f) * std::pow(std::sin(theta), 2.0f) + std::pow(b, 2.0f) * std::pow(std::cos(theta), 2.0f));
    };

    const QVector2D spacing{absoluteSpacing + proportionalSpacing * dabSize};

    const float a = spacing.x() / 2.0f;
    const float b = spacing.y() / 2.0f;
    const float theta = std::atan2(end.pos.y() - start.pos.y(), end.pos.x() - start.pos.x());
    float increment = ellipsePolar(a, b, theta + ((2.0f * pi<float>) / 360.0f) * -start.quaternion.scalar()) * 2.0f;

    const float incrementScale = 1.0;
    float pos = offset * increment;
    if (pos == 0.0f) {
        output.add(start);
        pos += increment;
    }
    const QVector2D delta = end.pos - start.pos;
    const float length = delta.length();
    while (pos < length) {
        output.add(Stroke::interpolate(start, end, pos / length));
        pos += std::max(increment, 1.0f);
        increment *= incrementScale;
    }

    return (pos - length) / increment;
}

void Editor::drawDab(const Brush::Dab &dab, const Colour &colour, BufferNode &node, const QVector2D worldPos)
{
    Q_ASSERT( m_editingContext.bufferNodeContext(&node));

    const Traversal::State &state = m_editingContext.states().value(&node);

    const QMatrix4x4 &transform = QMatrix4x4(state.transform);
    QMatrix4x4 spaceTransform;
    switch (dab.space) {
    case Space::Object: {
        spaceTransform = QMatrix4x4();
    } break;
    case Space::ObjectAspectCorrected: {
        spaceTransform = node.pixelAspectRatioTransform().inverted();
    } break;
    case Space::World: {
        spaceTransform = transform.inverted();
    } break;
    case Space::View: {
        spaceTransform = (cameraTransform * transform).inverted();
    } break;
    }
    const QVector2D spaceOffset = QVector2D(spaceTransform.map(QVector3D(QVector2D(0.0f, 0.0f))));
    QMatrix4x4 spaceOffsetTransform;
    spaceOffsetTransform.translate(-spaceOffset);
    spaceTransform = spaceOffsetTransform * spaceTransform;
    const QVector2D objectSpacePos = QVector2D(transform.inverted().map(worldPos.toPointF()));
    QMatrix4x4 objectSpaceTransform;
    objectSpaceTransform.translate(objectSpacePos);

    const Buffer *const palette = m_editingContext.states()[&node].palette;
    EditingContext::BufferNodeContext *const bufferNodeContext = m_editingContext.bufferNodeContext(&node);
    bufferNodeContext->dabProgram->render(dab, colour, GfxPaint::viewportTransform(node.buffer.size()) * objectSpaceTransform * spaceTransform, &node.buffer, palette);
}

void Editor::setTransformMode(const TransformMode transformMode)
{
    if (this->m_transformMode != transformMode) {
        this->m_transformMode = transformMode;
        emit transformModeChanged(this->m_transformMode);
    }
}

void Editor::setTransform(const QMatrix4x4 &transform)
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
    scene.render(nullptr, false, nullptr, QMatrix4x4(), &m_editingContext.states());

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
