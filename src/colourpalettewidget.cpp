#include "colourpalettewidget.h"

#include "application.h"

namespace GfxPaint {

ColourPaletteWidget::ColourPaletteWidget(QWidget *const parent) :
    RenderedWidget(parent),
    m_columnCount(16), m_fitColumnCount(false),
    m_swatchSize(16, 16), m_fitSwatchSize(true),
    program(nullptr), pickProgram(nullptr), selectionProgram(nullptr),
    m_palette(nullptr), m_selection(nullptr), m_ordering(nullptr)
{
    updatePaletteLayout();
}

ColourPaletteWidget::~ColourPaletteWidget()
{
    ContextBinder contextBinder(&qApp->renderManager.context, &qApp->renderManager.surface);
    delete program;
    delete pickProgram;
    delete selectionProgram;
    delete m_selection;
    delete m_ordering;
}

QSize ColourPaletteWidget::sizeHint() const { return QSize(cells().width() * m_swatchSize.width(), cells().height() * m_swatchSize.height()); }

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
    update();
}

void ColourPaletteWidget::updatePaletteLayout()
{
    setSizePolicy(m_fitColumnCount || m_fitSwatchSize ? QSizePolicy::Preferred : QSizePolicy::Fixed,
                  QSizePolicy::Fixed);
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

void ColourPaletteWidget::processMouseEvent(QMouseEvent * const event)
{
    if (event->buttons() & Qt::LeftButton) {
        if (m_palette) {
            const QVector2D pos = QVector2D((float)clamp(0.0f, 1.0f, (float)event->pos().x() / (float)width()),
                                            (float)clamp(0.0f, 1.0f, (float)event->pos().y() / (float)height()));
            Colour colour = pickProgram->pick(m_palette, cells(), pos);
            // TODO: painting with valid index screwy
            colour.index = INDEX_INVALID;///////////////////////////////////////////////////
            emit colourPicked(colour);
            event->accept();
            return;
        }
    }

    event->ignore();
}

void ColourPaletteWidget::initializeGL()
{
    RenderedWidget::initializeGL();

    setPalette(m_palette);
}

void ColourPaletteWidget::render()
{
    if (m_palette) {
        program->render(m_palette, cells(), RenderManager::unitToClipTransform, widgetBuffer);
    }
}

int ColourPaletteWidget::heightForWidth(const int width) const {
    const QSize actualCells = cellsForWidth(width);
    const float actualSwatchWidth = (m_fitSwatchSize ? (float)width / (float)actualCells.width() : m_swatchSize.width());
    const float actualSwatchHeight = (actualSwatchWidth / (float)m_swatchSize.width()) * (float)m_swatchSize.height();
    return (int)std::ceil(actualCells.height() * actualSwatchHeight);
}

QSize ColourPaletteWidget::cells() const {
    return cellsForWidth(width());
}

} // namespace GfxPaint
