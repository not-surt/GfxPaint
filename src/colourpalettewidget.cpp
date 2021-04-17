#include "colourpalettewidget.h"

#include "application.h"

namespace GfxPaint {

ColourPaletteWidget::ColourPaletteWidget(QWidget *const parent) :
    RenderedWidget(parent),
    m_columnCount(16), m_fitColumnCount(false),
    m_swatchSize(16, 16), m_fitSwatchSize(true),
    leftIndex(INDEX_INVALID), rightIndex(INDEX_INVALID),
    dragStartIndex(INDEX_INVALID), dragEndIndex(INDEX_INVALID),
    program(nullptr), markerProgram(nullptr), pickProgram(nullptr), selectionProgram(nullptr),
    m_palette(nullptr), m_selection(nullptr), m_ordering(nullptr)
{
    updatePaletteLayout();
}

ColourPaletteWidget::~ColourPaletteWidget()
{
    ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
    delete program;
    delete markerProgram;
    delete pickProgram;
    delete selectionProgram;
    delete m_selection;
    delete m_ordering;
}

QSize ColourPaletteWidget::sizeHint() const { return QSize(cells().width() * m_swatchSize.width(), cells().height() * m_swatchSize.height()); }

bool ColourPaletteWidget::event(QEvent *const event)
{
    // Handle input event
    bool handled = false;
    if (m_palette && (
            event->type() == QEvent::MouseButtonPress ||
            event->type() == QEvent::MouseMove ||
            event->type() == QEvent::MouseButtonRelease)) {
        const Index oldDragStartIndex = dragStartIndex;
        const Index oldDragEndIndex = dragEndIndex;
        const QMouseEvent *const mouseEvent = static_cast<QMouseEvent *>(event);
        const QVector2D pos = QVector2D((float)clamp(0.0f, 1.0f, (float)mouseEvent->pos().x() / (float)width()),
                                        (float)clamp(0.0f, 1.0f, (float)mouseEvent->pos().y() / (float)height()));
        const int cellX = floor(cells().width() * pos.x());
        const int cellY = floor(cells().height() * pos.y());
        const int index = clamp(0, m_palette->width() - 1, cellX + cellY * cells().width());
        if (mouseEvent->buttons() & Qt::LeftButton ||
            mouseEvent->buttons() & Qt::RightButton) {
            switch (event->type()) {
            case QEvent::MouseButtonPress: {
                dragStartIndex = index;
                handled = true;
            } break;
            case QEvent::MouseMove: {
                handled = true;
            } break;
            case QEvent::MouseButtonRelease: {
                dragEndIndex = index;
                handled = true;
            } break;
            default: {
            }
            }
            if (dragStartIndex != oldDragStartIndex || dragEndIndex != oldDragEndIndex) {
                ContextBinder binder(&qApp->renderManager.context, &qApp->renderManager.surface);
                Buffer workBuffer(*m_selection);
                workBuffer.copy(*m_selection);
            }
            if (mouseEvent->buttons() & Qt::LeftButton) leftIndex = index;
            if (mouseEvent->buttons() & Qt::RightButton) rightIndex = index;
            Colour colour = pickProgram->pick(m_palette, cells(), pos);
            // TODO: painting with valid index screwy
            colour.index = INDEX_INVALID;///////////////////////////////////////////////////
            emit colourPicked(colour);
            qDebug() << index << dragStartIndex << dragEndIndex;////////////////////////
        }
    }

    if (handled) {
        event->accept();
        return true;
    }
    else {
        return QOpenGLWidget::event(event);
    }
}
void ColourPaletteWidget::setPalette(const Buffer *const palette)
{
    m_palette = palette;
    QList<Program *> oldPrograms = {program, selectionProgram, pickProgram};
    ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
    if (m_palette) {
        delete m_selection;
        m_selection = new Buffer(m_palette->size(), BufferData::Format(BufferData::Format::ComponentType::UInt, 1, 1));
        m_selection->clearUInt();
        delete m_ordering;
        m_ordering = new Buffer(m_palette->size(), BufferData::Format(BufferData::Format::ComponentType::UInt, 4, 1));
        m_ordering->clearUInt();
        program = new ColourPaletteProgram(RenderedWidget::format, 0, m_palette->format());
        selectionProgram = new ColourPaletteProgram(RenderedWidget::format, 0, m_selection->format());
        pickProgram = new ColourPalettePickProgram(m_palette->format());
    }
    else {
        program = nullptr;
        selectionProgram = nullptr;
        pickProgram = nullptr;
    }
    qDeleteAll(oldPrograms);
//    update();
    updateGeometry();
}

void ColourPaletteWidget::updatePaletteLayout()
{
    setSizeIncrement(m_swatchSize);
    setSizePolicy((m_fitColumnCount || m_fitSwatchSize) ? QSizePolicy::Preferred : QSizePolicy::Fixed,
                  (m_fitSwatchSize) ? QSizePolicy::Preferred : QSizePolicy::Fixed);
    updateGeometry();
}

void ColourPaletteWidget::setColumnCount(const int columnCount)
{
    m_columnCount = columnCount;
    updatePaletteLayout();
}

void ColourPaletteWidget::setFitColumnCount(const bool fitColumnCount)
{
    m_fitColumnCount = fitColumnCount;
    updatePaletteLayout();
}

void ColourPaletteWidget::setSwatchSize(const QSize &swatchSize)
{
    m_swatchSize = swatchSize;
    updatePaletteLayout();
}

void ColourPaletteWidget::setFitSwatchSize(const bool fitSwatchSize)
{
    m_fitSwatchSize = fitSwatchSize;
    updatePaletteLayout();
}

void ColourPaletteWidget::initializeGL()
{
    RenderedWidget::initializeGL();

    {
        ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
        QList<Program *> oldPrograms = {markerProgram};
        markerProgram = new ModelProgram(RenderedWidget::format, false, Buffer::Format(), 0, RenderManager::composeModeDefault);
        qDeleteAll(oldPrograms);
    }
    setPalette(m_palette);
}

void ColourPaletteWidget::render()
{
    if (m_palette) {
        const QSize cells = this->cells();
        const QVector2D cellSize = QVector2D(1.0f / (float)cells.width(), 1.0f / (float)cells.height());
        QMatrix4x4 markerTransform;

        program->render(m_palette, cells, RenderManager::unitToClipTransform, widgetBuffer);

        const QPoint leftCell = QPoint(leftIndex % cells.width(), leftIndex / cells.width());
        markerTransform = RenderManager::unitToClipTransform;
        markerTransform.translate((float)leftCell.x() * cellSize.x(), (float)leftCell.y() * cellSize.y());
        markerTransform.scale(cellSize.x(), cellSize.y());
        Model *const markerModel = qApp->renderManager.models["paletteMouseMarker"];
        markerProgram->render(markerModel, {}, markerTransform, widgetBuffer, nullptr);

        const QPoint rightCell = QPoint(rightIndex % cells.width(), rightIndex / cells.width());
        markerTransform = RenderManager::unitToClipTransform;
        markerTransform.translate((float)(rightCell.x() + 1) * cellSize.x(), (float)(rightCell.y() + 1) * cellSize.y());
        markerTransform.rotate(180.0f, QVector3D(0.0f, 0.0f, 1.0f));
        markerTransform.scale(cellSize.x(), cellSize.y());
        markerProgram->render(markerModel, {}, markerTransform, widgetBuffer, nullptr);
    }
}

int ColourPaletteWidget::heightForWidth(const int width) const {
    qDebug() << width << this->width();//////////////
    const QSize actualCells = cellsForWidth(width);
    const float actualSwatchWidth = (m_fitSwatchSize ? (float)width / (float)actualCells.width() : m_swatchSize.width());
    const float actualSwatchHeight = (actualSwatchWidth / (float)m_swatchSize.width()) * (float)m_swatchSize.height();
    return (int)std::ceil(actualCells.height() * actualSwatchHeight);
}

QSize ColourPaletteWidget::cells() const {
    return cellsForWidth(width());
}

} // namespace GfxPaint
