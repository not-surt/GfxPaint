#include "editor.h"

#include <QMouseEvent>
#include <QtMath>
#include "application.h"
#include "utils.h"

namespace GfxPaint {

Editor::Editor(Scene &scene, QWidget *parent) :
    RenderedWidget(parent),
    scene(scene), model(*qApp->documentManager.documentModel(&scene)),
    m_editingContext(scene),
    cameraTransform(), transformMode(),
    inputState(this)
{
    setMouseTracking(true);
    setFocusPolicy(Qt::WheelFocus);

    QObject::connect(&m_editingContext.selectionModel(), &QItemSelectionModel::selectionChanged, this, &Editor::updateContext);
}

Editor::Editor(const Editor &other) :
    RenderedWidget(other.parentWidget()),
    scene(other.scene), model(other.model),
    m_editingContext(other.scene),
    cameraTransform(other.cameraTransform), transformMode(other.transformMode),
    inputState(this)
{
    setMouseTracking(true);
    setFocusPolicy(Qt::WheelFocus);
}

Editor::~Editor()
{
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
    emit transformModeChanged(this->transformMode);
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

void Editor::setColour(const QColor &colour)
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
        BufferNode *const bufferNode = dynamic_cast<BufferNode *>(node);
        if (m_editingContext.workBuffers().contains(node) && bufferNode) {
            m_editingContext.workBuffers()[node]->copy(bufferNode->buffer);
            // Draw on-canvas brush preview
            if (inputState.machine.configuration().contains(&inputState.mouseIn)) {
                drawDab(m_editingContext.brush().dab, m_editingContext.colour(), *bufferNode, cameraTransform.inverted().map(QPointF(mouseTransform.map(mapFromGlobal(QCursor::pos())))));
            }
        }
    }

    // Draw scene
    widgetBuffer->bindFramebuffer();
    scene.render(*widgetBuffer, false, Buffer({1, 1}, Buffer::Format(Buffer::Format::ComponentType::UInt, 1, 4)), cameraTransform * viewportTransform, &m_editingContext.states());

    // Undraw on-canvas brush preview
    for (auto index : m_editingContext.selectionModel().selectedRows()) {
        Node *node = static_cast<Node *>(index.internalPointer());
        BufferNode *const bufferNode = dynamic_cast<BufferNode *>(node);
        if (m_editingContext.workBuffers().contains(node) && bufferNode) {
            if (inputState.machine.configuration().contains(&inputState.mouseIn)) {
                bufferNode->buffer.copy(*m_editingContext.workBuffers()[node]);
            }
        }
    }
}

void Editor::rotateScaleAtOrigin(QTransform &transform, const qreal rotation, const qreal scaling, const QPointF origin)
{
    const QPointF pointBefore = (transform.inverted()).map(origin);
    transform *= QTransform().scale(scaling, scaling) * QTransform().rotate(rotation);
    const QPointF pointAfter = (transform.inverted()).map(origin);
    const QPointF offset = pointAfter - pointBefore;
    transform = QTransform().translate(offset.x(), offset.y()) * transform;
}

QTransform Editor::transformPointToPoint(const QPointF origin, const QPointF from, const QPointF to)
{
    const QVector2D fromVector = QVector2D(from - origin);
    const QVector2D toVector = QVector2D(to - origin);
    const qreal scaling = toVector.length() / fromVector.length();
    const qreal rotation = qRadiansToDegrees(atan2(toVector.y(), toVector.x()) - atan2(fromVector.y(), fromVector.x()));
    QTransform transform;
    rotateScaleAtOrigin(transform, rotation, scaling, origin);
    return transform;
}

qreal Editor::strokeSegmentDabs(const QPointF start, const QPointF end, const  qreal spacing, const qreal offset, QList<QPointF> &output) {
    const QPointF delta(end.x() - start.x(), end.y() - start.y());
    const qreal length = hypot(delta.x(), delta.y());
    const QPointF step(delta.x() / length, delta.y() / length);
    qreal pos, i;
    for (pos = offset, i = 0; pos < length; pos += spacing, ++i) {
        output.append(QPointF(start.x() + pos * step.x(), start.y() + pos * step.y()));
    }
    return pos - length;
}

void Editor::drawDab(const Dab &dab, const QColor &colour, BufferNode &node, const QPointF worldPos)
{    
    Q_ASSERT(m_editingContext.dabPrograms().contains(&node));

    const Traversal::State &state = m_editingContext.states().value(&node);

    QTransform transform = state.transform;
    const QPointF objectSpacePos = transform.inverted().map(worldPos);
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

    ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
    node.buffer.bindFramebuffer();
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_BLEND);
//    const Buffer *const palette = m_editingContext.states()[&node].palette;
    const Buffer *const palette = &m_editingContext.states()[&node].palette;
    qDebug() << "STATE PALETTE" << palette << palette->size() << static_cast<int>(palette->format().componentType) << palette->format().componentSize << palette->format().componentCount << palette->format().scale() << palette->isNull(); ////////////////////////////////////////////
    m_editingContext.dabPrograms()[&node]->render(dab, colour, spaceTransform * QTransform().translate(objectSpacePos.x(), objectSpacePos.y()) * GfxPaint::viewportTransform(node.buffer.size()), &node.buffer, palette);
}

void Editor::setTransformMode(const TransformMode transformMode)
{
    if (this->transformMode != transformMode) {
        this->transformMode = transformMode;
        emit transformModeChanged(this->transformMode);
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
        m_editingContext.states().insert(node, {});
    }
    update(); // Rebuild states, full traversal render bad!

    m_editingContext.updatePrograms();
    m_editingContext.updateWorkBuffers();
}

bool Editor::handleMouseEvent(const Qt::KeyboardModifiers modifiers, const Qt::MouseButton button, const QPoint pos)
{
    const QPointF oldMouseViewportPos = mouseTransform.map(inputState.oldPos);
    const QPointF oldMouseWorldPos = cameraTransform.inverted().map(QPointF(oldMouseViewportPos));
    const QPointF mouseViewportPos = mouseTransform.map(pos);
    const QPointF mouseWorldPos = cameraTransform.inverted().map(QPointF(mouseViewportPos));

    const QSet<QAbstractState *> states = inputState.machine.configuration();
    if (states.contains(&inputState.primaryTool) && states.contains(&inputState.standardTool)) {
        for (auto index : m_editingContext.selectionModel().selectedRows()) {
            Node *node = static_cast<Node *>(index.internalPointer());
            const Traversal::State &state = m_editingContext.states().value(node);
            BufferNode *const bufferNode = dynamic_cast<BufferNode *>(node);
            if (bufferNode) {
                QList<QPointF> points;
                strokeSegmentDabs(oldMouseWorldPos, mouseWorldPos, 16.0, 0.0, points);
                for (auto point : points) {
                    drawDab(m_editingContext.brush().dab, m_editingContext.colour(), *bufferNode, point);
                }
            }
        }
        update();
    }
    else if (states.contains(&inputState.primaryTool) && states.contains(&inputState.alternateTool)) {
        for (auto index : m_editingContext.selectionModel().selectedRows()) {
            Node *node = static_cast<Node *>(index.internalPointer());
            const Traversal::State &state = m_editingContext.states().value(node);
            BufferNode *const bufferNode = dynamic_cast<BufferNode *>(node);
            if (bufferNode) {
                const QPointF mouseBufferPos = state.transform.inverted().map(mouseWorldPos);
                setColour(m_editingContext.colourPickPrograms()[bufferNode]->pick(&bufferNode->buffer, mouseBufferPos));
            }
        }
    }
    else if (states.contains(&inputState.secondaryTool) && states.contains(&inputState.standardTool)) {
    }
    else if (states.contains(&inputState.secondaryTool) && states.contains(&inputState.alternateTool)) {
    }
    else if (states.contains(&inputState.transform) && states.contains(&inputState.standardTool)) {
        if (m_editingContext.selectionModel().selectedRows().isEmpty()) {
            const QPointF translation = mouseWorldPos - oldMouseWorldPos;
            const QTransform translationTransform = QTransform().translate(translation.x(), translation.y());
            if (transformMode == TransformMode::View) {
                cameraTransform = translationTransform * cameraTransform;
                emit transformChanged(cameraTransform);
            }
        }
        for (auto index : m_editingContext.selectionModel().selectedRows()) {
            Node *node = static_cast<Node *>(index.internalPointer());
            const Traversal::State &state = m_editingContext.states().value(node);
            const QPointF translation = mouseWorldPos - oldMouseWorldPos;
            const QTransform translationTransform = QTransform().translate(translation.x(), translation.y());
            if (transformMode == TransformMode::View) {
                cameraTransform = translationTransform * cameraTransform;
                emit transformChanged(cameraTransform);
            }
            else if (transformMode == TransformMode::Object) {
                SpatialNode *const spatialNode = dynamic_cast<SpatialNode *>(node);
                if (spatialNode) {
                    const QPointF translation = state.parentTransform.inverted().map(mouseWorldPos) - state.parentTransform.inverted().map(oldMouseWorldPos);
                    spatialNode->setTransform(spatialNode->transform() * QTransform().translate(translation.x(), translation.y()));
                }
            }
            else if (transformMode == TransformMode::Brush) {

            }
        }
        update();
    }
    else if (states.contains(&inputState.transform) && states.contains(&inputState.alternateTool)) {
        if (m_editingContext.selectionModel().selectedRows().isEmpty()) {
            if (transformMode == TransformMode::View) {
                cameraTransform *= transformPointToPoint(QPointF(0., 0.), oldMouseViewportPos, mouseViewportPos);
                emit transformChanged(cameraTransform);
            }
        }
        for (auto index : m_editingContext.selectionModel().selectedRows()) {
            Node *node = static_cast<Node *>(index.internalPointer());
            const Traversal::State &state = m_editingContext.states().value(node);
            if (transformMode == TransformMode::View) {
                cameraTransform *= transformPointToPoint(QPointF(0., 0.), oldMouseViewportPos, mouseViewportPos);
                emit transformChanged(cameraTransform);
            }
            else if (transformMode == TransformMode::Object) {
                SpatialNode *const spatialNode = dynamic_cast<SpatialNode *>(node);
                if (spatialNode) {
                    const QTransform transformSpace = (state.parentTransform * cameraTransform).inverted();
                    spatialNode->setTransform(spatialNode->transform() * transformPointToPoint(transformSpace.map(QPointF(0., 0.)), transformSpace.map(oldMouseViewportPos), transformSpace.map(mouseViewportPos)));
                }
            }
            else if (transformMode == TransformMode::Brush) {

            }
        }
        update();
    }
    else if (inputState.machine.configuration().contains(&inputState.mouseIn)) {
        update();
    }

    inputState.oldPos = pos;

    return true;
}

void Editor::mousePressEvent(QMouseEvent *event)
{
    inputState.oldPos = event->pos();
    event->setAccepted(handleMouseEvent(event->modifiers(), event->button(), event->pos()));
}

void Editor::mouseReleaseEvent(QMouseEvent *event)
{
    event->setAccepted(handleMouseEvent(event->modifiers(), event->button(), event->pos()));
}

void Editor::mouseMoveEvent(QMouseEvent *event)
{
    for (auto button : {Qt::NoButton, Qt::LeftButton, Qt::RightButton, Qt::MidButton}) {
        if (event->buttons() & button || event->buttons() == button)
            event->setAccepted(handleMouseEvent(event->modifiers(), button, event->pos()));
    }
}

void Editor::wheelEvent(QWheelEvent *event)
{
    const QPointF mouseViewportPos = mouseTransform.map(event->posF());
    const QPointF mouseWorldPos = cameraTransform.inverted().map(QPointF(mouseViewportPos));
    const qreal delta = event->angleDelta().y() / (15.0 * 8.0);
    const qreal scaling = pow(2, delta);
    const qreal rotation = -15.0 * delta;
    rotateScaleAtOrigin(cameraTransform,
                       (event->modifiers() == Qt::ShiftModifier) ? rotation : 0.0,
                       (event->modifiers() == Qt::NoModifier) ? scaling : 1.0,
                       mouseViewportPos);
    emit transformChanged(cameraTransform);
    update();
    event->accept();
}

void Editor::keyPressEvent(QKeyEvent *event)
{
    /*if (event->key() == Qt::Key_Space && !event->isAutoRepeat()) {
        qDebug() << "keyPressEvent";
        //event->ignore();
    }
    else */return RenderedWidget::keyPressEvent(event);
}

void Editor::keyReleaseEvent(QKeyEvent *event)
{
    /*if (event->key() == Qt::Key_Space && !event->isAutoRepeat()) {
        qDebug() << "keyReleaseEvent";
        //event->ignore();
    }
    else */return RenderedWidget::keyPressEvent(event);
}

} // namespace GfxPaint
