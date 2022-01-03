#-------------------------------------------------
#
# Project created by QtCreator 2017-04-14T20:53:18
#
#-------------------------------------------------

QT_REQUIRED_MAJOR_VERSION = 6
QT_REQUIRED_MINOR_VERSION = 0
QT_REQUIRED_PATCH_VERSION = 0
QT_REQUIRED_VERISION = $${QT_REQUIRED_MAJOR_VERSION}.$${QT_REQUIRED_MINOR_VERSION}.$${QT_REQUIRED_PATCH_VERSION}

!versionAtLeast(QT_VERSION, $$QT_REQUIRED_VERISION) {
    message("Cannot use Qt $${QT_VERSION}")
    error("Use Qt $${QT_REQUIRED_VERISION} or newer")
}

QT += core gui widgets openglwidgets

TARGET = GfxPaint
VERSION = 0.0.0
TEMPLATE = app

DEFINES += APP_VERSION=$$VERSION

APP_GIT_REVISION = $$system(git rev-parse --short=8 HEAD)
APP_GIT_TAG = $$system(git describe --tags --abbrev=0)

DEFINES += APP_GIT_REVISION=$$APP_GIT_REVISION APP_GIT_TAG=$$APP_GIT_TAG

CONFIG += c++2a

QMAKE_CXXFLAGS += -pedantic -pedantic-errors
QMAKE_CXXFLAGS_WARN_ON += -Wno-unused-parameter -Wno-unused-variable

DEPRECATED_MAJOR_VERSION = $$format_number($${QT_REQUIRED_MAJOR_VERSION}, width=2 zeropad)
DEPRECATED_MINOR_VERSION = $$format_number($${QT_REQUIRED_MINOR_VERSION}, width=2 zeropad)
DEPRECATED_PATCH_VERSION = $$format_number($${QT_REQUIRED_PATCH_VERSION}, width=2 zeropad)
DEPRECATED_VERSION = 0x$${DEPRECATED_MAJOR_VERSION}$${DEPRECATED_PATCH_VERSION}$${DEPRECATED_PATCH_VERSION}
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=$$DEPRECATED_VERSION

#PRECOMPILED_HEADER = stable.h

VPATH += src/ thirdparty/src/
INCLUDEPATH += $$PWD/src/ $$PWD/thirdparty/src/

SOURCES +=\
    application.cpp \
    brush.cpp \
    brushviewwidget.cpp \
    buffer.cpp \
    colourcomponentsplanewidget.cpp \
    colourplanewidget.cpp \
    coloursliderswidget.cpp \
    dabeditorwidget.cpp \
    dockwidget.cpp \
    dockwidgettitlebarwidget.cpp \
    documentmanager.cpp \
    documentsmodel.cpp \
    editingcontext.cpp \
    mainwindow.cpp \
    multitoolbutton.cpp \
    node.cpp \
    opengl.cpp \
    program.cpp \
    renderedwidget.cpp \
    rendermanager.cpp \
    scene.cpp \
    sessionmanager.cpp \
    tileseticonmanager.cpp \
    strokeeditorwidget.cpp \
    qtpropertybrowser/qtbuttonpropertybrowser.cpp \
    qtpropertybrowser/qteditorfactory.cpp \
    qtpropertybrowser/qtgroupboxpropertybrowser.cpp \
    qtpropertybrowser/qtpropertybrowser.cpp \
    qtpropertybrowser/qtpropertybrowserutils.cpp \
    qtpropertybrowser/qtpropertymanager.cpp \
    qtpropertybrowser/qttreepropertybrowser.cpp \
    qtpropertybrowser/qtvariantproperty.cpp \
    simplecpp/simplecpp.cpp \
    transformeditorwidget.cpp \
    transformmodel.cpp \
    types.cpp \
    utils.cpp \
    workbuffermanager.cpp \
    sessioneditorwidget.cpp \
    editor.cpp \
    newbufferdialog.cpp \
    scenemodel.cpp \
    scenetreewidget.cpp \
    model.cpp \
    nodeeditorwidget.cpp \
    tool.cpp \
    stroke.cpp \
    colourpalettewidget.cpp \
    paletteeditorwidget.cpp

HEADERS  += \
    application.h \
    brush.h \
    brushviewwidget.h \
    buffer.h \
    colourcomponentsplanewidget.h \
    colourplanewidget.h \
    coloursliderswidget.h \
    dabeditorwidget.h \
    dockwidget.h \
    dockwidgettitlebarwidget.h \
    documentmanager.h \
    documentsmodel.h \
    editingcontext.h \
    mainwindow.h \
    multitoolbutton.h \
    node.h \
    opengl.h \
    program.h \
    renderedwidget.h \
    rendermanager.h \
    scene.h \
    sessionmanager.h \
    src/tileseticonmanager.h \
    strokeeditorwidget.h \
    frozen/algorithm.h \
    frozen/bits/algorithms.h \
    frozen/bits/basic_types.h \
    frozen/bits/constexpr_assert.h \
    frozen/bits/defines.h \
    frozen/bits/elsa.h \
    frozen/bits/exceptions.h \
    frozen/bits/pmh.h \
    frozen/bits/version.h \
    frozen/map.h \
    frozen/random.h \
    frozen/set.h \
    frozen/string.h \
    frozen/unordered_map.h \
    frozen/unordered_set.h \
    qtpropertybrowser/qtbuttonpropertybrowser.h \
    qtpropertybrowser/qteditorfactory.h \
    qtpropertybrowser/qtgroupboxpropertybrowser.h \
    qtpropertybrowser/qtpropertybrowser.h \
    qtpropertybrowser/qtpropertybrowserutils_p.h \
    qtpropertybrowser/qtpropertymanager.h \
    qtpropertybrowser/qttreepropertybrowser.h \
    qtpropertybrowser/qtvariantproperty.h \
    simplecpp/simplecpp.h \
    transformeditorwidget.h \
    transformmodel.h \
    types.h \
    utils.h \
    workbuffermanager.h \
    sessioneditorwidget.h \
    editor.h \
    newbufferdialog.h \
    scenemodel.h \
    scenetreewidget.h \
    model.h \
    nodeeditorwidget.h \
    tool.h \
    stroke.h \
    colourpalettewidget.h \
    paletteeditorwidget.h

FORMS    += \
    colourplanewidget.ui \
    coloursliderswidget.ui \
    mainwindow.ui \
    dabeditorwidget.ui \
    strokeeditorwidget.ui \
    transformeditorwidget.ui \
    sessioneditorwidget.ui \
    newbufferdialog.ui \
    scenetreewidget.ui \
    nodeeditorwidget.ui \
    paletteeditorwidget.ui

RESOURCES += \
    icon.qrc \
    thirdparty/fonts/thirdpartyfonts.qrc \
    thirdparty/src/qtpropertybrowser/qtpropertybrowser.qrc \
    thirdparty/stylesheets/qdarkstyle/style.qrc \
    thirdparty/stylesheets/darkorange/darkorange.qrc \
    thirdparty/shaders/thirdpartyshaders.qrc \
    shaders/shaders.qrc

DISTFILES += \
    GfxPaint.ico \
    LICENSE \
    README.md \
    TODO \
    screenshot.png \

RC_ICONS = GfxPaint.ico
