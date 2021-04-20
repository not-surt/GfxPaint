#-------------------------------------------------
#
# Project created by QtCreator 2017-04-14T20:53:18
#
#-------------------------------------------------

QT       += core gui widgets
greaterThan(QT_MAJOR_VERSION, 5) {
    QT   += openglwidgets
}
#QT       += gamepad

TARGET = GfxPaint
TEMPLATE = app

CONFIG += c++2a

QMAKE_CXXFLAGS += -pedantic -pedantic-errors -Wno-unused-parameter -Wno-unused-variable

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x051500    # disables all the APIs deprecated before Qt 5.15.0
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

VPATH += src/ thirdparty/
INCLUDEPATH += src/ thirdparty/include/

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
    strokeeditorwidget.cpp \
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
    thirdparty/fonts/thirdpartyfonts.qrc \
    thirdparty/stylesheets/qdarkstyle/style.qrc \
    thirdparty/stylesheets/darkorange/darkorange.qrc \
    thirdparty/shaders/thirdpartyshaders.qrc \
    shaders/shaders.qrc

DISTFILES += \
    LICENSE \
    README.md \
    TODO
