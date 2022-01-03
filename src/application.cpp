#include "application.h"

#include <QtGlobal>
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QSettings>
#include <QMessageBox>
#include <QPainter>
#include <QImageReader>
#include <QImageWriter>
#include <QFileInfo>
#include <QSettings>
#include <QThread>
#include <QStyleFactory>
#include <QStyle>
#include <QDir>
#include <QOpenGLShader>
#include <QOpenGLShaderProgram>
#include <QMessageBox>
#include <QStandardPaths>
#include <QCommandLineParser>
#include <QProgressDialog>
#include <QCommonStyle>
#include <QFontDatabase>

#include "mainwindow.h"
#include "utils.h"
#include "scene.h"

namespace GfxPaint {

const std::map<QImage::Format, QImage::Format> Application::qImageConversion = {
    {QImage::Format_Mono, QImage::Format_Indexed8},
    {QImage::Format_Grayscale8, QImage::Format_Indexed8},
    {QImage::Format_RGB32, QImage::Format_ARGB32},
    {QImage::Format_ARGB32_Premultiplied, QImage::Format_ARGB32},
};

const std::map<QImage::Format, Buffer::Format> Application::qImageToBuffer = {
    {QImage::Format_Indexed8, Buffer::Format(BufferData::Format::ComponentType::UInt, 1, 1)},
    {QImage::Format_ARGB32, Buffer::Format(BufferData::Format::ComponentType::UInt, 1, 4)}
};

const std::map<Buffer::Format, QImage::Format> Application::bufferToQImage = {
    {Buffer::Format(BufferData::Format::ComponentType::UInt, 1, 1), QImage::Format_Indexed8},
    {Buffer::Format(BufferData::Format::ComponentType::UInt, 1, 4), QImage::Format_ARGB32},
};

const QString Application::sessionExtension = "session.gfx";

const QList<Application::NodeInfo> Application::nodeInfo = {
    {"Node", &Node::create, nullptr, nullptr},
    {"Spatial", &SpatialNode::create, nullptr, nullptr},
    {"Buffer", nullptr, &BufferNode::createFromFile, &BufferNode::createFromDialog},
    {"Palette", &PaletteNode::create, &PaletteNode::createFromFile, nullptr},
};

Application::Application(int &argc, char **argv)
    : QApplication(argc, argv),
      renderManager(),
      workBufferManager(),
      sessionManager(), documentManager(),
      m_gitRevision(),
      m_recentSessions(), m_recentFiles(),
      styles(), m_styleActions(this), palettes(), m_paletteActions(this), stylesheets(), m_stylesheetActions(this),
      m_reopenSessionAtStartup(true), m_saveSessionAtExit(true), timer(),
      m_iconFont()
{
    timer.start();

    // Application settings
    setOrganizationName("surt");
    setOrganizationDomain("surt");
    static const QString name = "GfxPaint";
    setApplicationName(name);
    setApplicationDisplayName(name);
    setApplicationVersion(APP_VERSION);
    setGitRevision(APP_GIT_REVISION);
    setDesktopFileName(name + ".desktop");
    QSettings::setDefaultFormat(QSettings::IniFormat);
    setQuitOnLastWindowClosed(false);
//    setFallbackSessionManagementEnabled(false);

    QObject::connect(this, &QGuiApplication::commitDataRequest, this, &Application::commitData, Qt::DirectConnection);
    QObject::connect(this, &QGuiApplication::saveStateRequest, this, &Application::saveState, Qt::DirectConnection);

    const int iconFontId = QFontDatabase::addApplicationFont(":/fonts/fontawesome-free-5.15.3-desktop/otfs/Font Awesome 5 Free-Solid-900.otf");
    m_iconFont = QFont("Font Awesome 5 Free");

    const QStringList styleFactoryKeys = QStyleFactory::keys();
    styles["Default"] = style();
    for (const auto &key : styleFactoryKeys) {
        styles[key] = QStyleFactory::create(key);
    }
    const QList<QString> stylesKeys = styles.keys();
    for (const auto &key : stylesKeys) {
        QAction *const action = new QAction(key, this);
        action->setCheckable(true);
        action->setData(key);
        QObject::connect(action, &QAction::triggered, this, [this, key](){
            style()->setParent(nullptr);
            setStyle(styles.value(key));
        });
        m_styleActions.addAction(action);
    }
    m_styleActions.setExclusive(true);
    style()->setParent(nullptr);
    setStyle(styles["Default"]);

    palettes["Default"] = palette();
    palettes["Dark"] = QPalette(QColor(63, 63, 63), QColor(31, 31, 31));
    palettes["Grey"] = QPalette(QColor(95, 95, 95), QColor(63, 63, 63));
    palettes["Earth"] = QPalette(QColor(95, 79, 63), QColor(63, 47, 31));
    palettes["Moss"] = QPalette(QColor(63, 95, 79), QColor(31, 63, 47));
    palettes["Slate"] = QPalette(QColor(63, 79, 95), QColor(31, 47, 63));
    const QList<QString> palettesKeys = palettes.keys();
    for (const auto &key : palettesKeys) {
        QAction *const action = new QAction(key, this);
        action->setCheckable(true);
        action->setData(key);
        QObject::connect(action, &QAction::triggered, this, [this, key](){
            setPalette(palettes.value(key));
        });
        m_paletteActions.addAction(action);
    }
    m_paletteActions.setExclusive(true);
    setPalette(palettes["Default"]);

    const QVector<std::pair<QString, QString>> stylesheetFilenames = {
        {"QDarkStyle", ":/qdarkstyle/style.qss"},
        {"Dark Orange", ":/darkorange.qss"}
    };
    for (const auto &pair : stylesheetFilenames) {
        QFile file(pair.second);
        file.open(QFile::ReadOnly | QFile::Text);
        stylesheets[pair.first] = QTextStream(&file).readAll();
    }
    stylesheets["None"] = "";
    const QList<QString> stylesheetsKeys = stylesheets.keys();
    for (const auto &key : stylesheetsKeys) {
        QAction *const action = new QAction(key, this);
        action->setCheckable(true);
        action->setData(key);
        QObject::connect(action, &QAction::triggered, this, [this, key](){
            setStyleSheet(stylesheets.value(key));
        });
        m_stylesheetActions.addAction(action);
    }
    m_stylesheetActions.setExclusive(true);
    setStyleSheet(stylesheets["None"]);

    documentManager.registerDocumentType(&Scene::open, Scene::formats());

    QSettings settings;
    readSettings(settings);
}

Application::~Application()
{
    // Must turn off stylesheet before delete styles
    setStyleSheet("");
    // Ugly hack as QApplication needs to delete a style but don't want it deleting from style map
    style()->setParent(nullptr);
    setStyle(QStyleFactory::create(QStyleFactory::keys().at(0)));
    qDeleteAll(styles);
}

void Application::exit(const int returnCode)
{
    QSettings settings;
    writeSettings(settings);
    QGuiApplication::exit(returnCode);
}

void Application::updateRecentSessions(const QStringList &filenames)
{
    static const int maxRecentSessions = 16;
    for (const auto &filename : filenames) {
        m_recentSessions.removeAll(filename);
        m_recentSessions.prepend(filename);
    }
    while (m_recentSessions.length() > maxRecentSessions) m_recentSessions.removeLast();

    emit recentSessionsChanged();
}

void Application::updateRecentSessions(const QString &filename)
{
    updateRecentSessions(QStringList(filename));
}

const QStringList &Application::recentSessions() const
{
    return m_recentSessions;
}

QString Application::openImageFilters()
{
    return buildFilterString(QImageReader::supportedImageFormats(), "image");
}

QString Application::saveImageFilters()
{
    return buildFilterString(QImageWriter::supportedImageFormats(), "image");
}

void Application::updateRecentFiles(const QStringList &filenames)
{
    static const int maxRecentFiles = 16;
    for (const auto &filename : filenames) {
        m_recentFiles.removeAll(filename);
        m_recentFiles.prepend(filename);
    }
    while (m_recentFiles.length() > maxRecentFiles) m_recentFiles.removeLast();

    emit recentFilesChanged();
}

void Application::updateRecentFiles(const QString &filename)
{
    updateRecentFiles(QStringList(filename));
}

const QStringList &Application::recentFiles() const
{
    return m_recentFiles;
}

QList<QAction *> Application::styleActions()
{
    return m_styleActions.actions();
}

QList<QAction *> Application::paletteActions()
{
    return m_paletteActions.actions();
}

QList<QAction *> Application::stylesheetActions()
{
    return m_stylesheetActions.actions();
}

bool Application::saveSessionAtExit() const
{
    return m_saveSessionAtExit;
}

float Application::time() const
{
    return timer.elapsed() / 1000.0f;
}

void Application::commitData(QSessionManager &manager)
{
    // TODO
}

void Application::saveState(QSessionManager &manager)
{
    // TODO
}

void Application::setSaveSessionAtExit(const bool value)
{
    if (m_saveSessionAtExit != value) {
        m_saveSessionAtExit = value;
        emit saveSessionAtExitChanged(value);
    }
}

bool Application::reopenSessionAtStartup() const
{
    return m_reopenSessionAtStartup;
}

void Application::setReopenSessionAtStartup(const bool value)
{
    if (m_reopenSessionAtStartup != value) {
        m_reopenSessionAtStartup = value;
        emit reopenSessionAtStartupChanged(value);
    }
}

void Application::readSettings(QSettings &settings)
{
    QAction *defaultStyleAction = nullptr;
    for (auto &action : m_styleActions.actions()) {
        if (settings.contains("style")) action->setChecked(action->data() == settings.value("style"));
        if (action->text() == "Default") defaultStyleAction = action;
    }
    if (m_styleActions.checkedAction()) {
        setStyle(styles[settings.value("style").toString()]);
    }
    else if (defaultStyleAction) {
        defaultStyleAction->setChecked(true);
        setStyle(styles[m_styleActions.checkedAction()->data().toString()]);
    }

    QAction *defaultPaletteAction = nullptr;
    for (auto &action : m_paletteActions.actions()) {
        if (settings.contains("palette")) action->setChecked(action->data() == settings.value("palette"));
        if (action->text() == "Default") defaultPaletteAction = action;
    }
    if (m_paletteActions.checkedAction()) {
        setPalette(palettes[settings.value("palette").toString()]);
    }
    else if (defaultPaletteAction) {
        defaultPaletteAction->setChecked(true);
        setPalette(palettes[m_paletteActions.checkedAction()->data().toString()]);
    }

    QAction *defaultStylesheetAction = nullptr;
    for (auto &action : m_stylesheetActions.actions()) {
        if (settings.contains("stylesheet")) action->setChecked(action->data() == settings.value("stylesheet"));
        if (action->text() == "None") defaultStylesheetAction = action;
    }
    if (m_stylesheetActions.checkedAction()) {
        setStyleSheet(stylesheets[settings.value("stylesheet").toString()]);
    }
    else if (defaultStylesheetAction) {
        defaultStylesheetAction->setChecked(true);
        setStyleSheet(stylesheets[m_stylesheetActions.checkedAction()->data().toString()]);
    }

    const int recentSessionCount = settings.beginReadArray("recentSessions");
    for (int i = 0; i < recentSessionCount; ++i) {
        settings.setArrayIndex(i);
        m_recentSessions.append(settings.value("filename").toString());
    }
    settings.endArray();

    const int recentFileCount = settings.beginReadArray("recentFiles");
    for (int i = 0; i < recentFileCount; ++i) {
        settings.setArrayIndex(i);
        m_recentFiles.append(settings.value("filename").toString());
    }
    settings.endArray();

    if (settings.contains("reopenSessionAtStartup")) m_reopenSessionAtStartup = settings.value("reopenSessionAtStartup").toBool();
    if (settings.contains("saveSessionAtExit")) m_saveSessionAtExit = settings.value("saveSessionAtExit").toBool();
    if (settings.contains("lastSession")) sessionManager.setSessionFilename(settings.value("lastSession").toString());

    if (m_reopenSessionAtStartup && sessionManager.openSession(sessionManager.sessionFilename())) {}
    else sessionManager.newSession();
}

void Application::writeSettings(QSettings &settings)
{
    if (m_styleActions.checkedAction()) settings.setValue("style", m_styleActions.checkedAction()->data());
    if (m_paletteActions.checkedAction()) settings.setValue("palette", m_paletteActions.checkedAction()->data());
    if (m_stylesheetActions.checkedAction()) settings.setValue("stylesheet", m_stylesheetActions.checkedAction()->data());

    settings.remove("recentSessions");
    settings.beginWriteArray("recentSessions");
    for (int i = 0; i < m_recentSessions.size(); ++i) {
        settings.setArrayIndex(i);
        settings.setValue("filename", m_recentSessions.at(i));
    }
    settings.endArray();

    settings.remove("recentFiles");
    settings.beginWriteArray("recentFiles");
    for (int i = 0; i < m_recentFiles.size(); ++i) {
        settings.setArrayIndex(i);
        settings.setValue("filename", m_recentFiles.at(i));
    }
    settings.endArray();

    settings.setValue("reopenSessionAtStartup", m_reopenSessionAtStartup);
    settings.setValue("saveSessionAtExit", m_saveSessionAtExit);
    settings.setValue("lastSession", sessionManager.sessionFilename());

    if (m_saveSessionAtExit) {
        sessionManager.saveSession(sessionManager.sessionFilename());
    }
}

} // namespace GfxPaint

int main(int argc, char *argv[])
{
    QSurfaceFormat::setDefaultFormat(GfxPaint::RenderManager::defaultFormat());
    QGuiApplication::setAttribute(Qt::AA_ShareOpenGLContexts, true);
    QGuiApplication::setAttribute(Qt::AA_CompressHighFrequencyEvents, false);
    QGuiApplication::setAttribute(Qt::AA_CompressTabletEvents, false);
//    QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling, true);

    GfxPaint::Application app(argc, argv);

    QCommandLineParser parser;
    parser.setApplicationDescription(app.applicationDisplayName() + " - Image Editor");
    parser.addHelpOption();
    parser.addVersionOption();
    parser.addPositionalArgument("files", "Files to open.", "[files...]");
    parser.process(app);

    return app.exec();
}
