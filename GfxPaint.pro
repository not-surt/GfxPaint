#-------------------------------------------------
#
# Project created by QtCreator 2017-04-14T20:53:18
#
#-------------------------------------------------

QT       += core gui widgets #openglwidgets
#QT       += gamepad

TARGET = GfxPaint
TEMPLATE = app

CONFIG += c++17
#QMAKE_CXXFLAGS += -std=c++17

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x051500    # disables all the APIs deprecated before Qt 5.15.0

#INCLUDEPATH += thirdparty/libraries/glm

SOURCES +=\
    application.cpp \
    brush.cpp \
    brushviewwidget.cpp \
    buffer.cpp \
    colourplanewidget.cpp \
    dabeditorwidget.cpp \
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
    colourspacesliderswidget.cpp \
    model.cpp \
    colourcomponentsliderwidget.cpp \
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
    colourplanewidget.h \
    dabeditorwidget.h \
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
    transformeditorwidget.h \
    transformmodel.h \
    utils.h \
    workbuffermanager.h \
    sessioneditorwidget.h \
    editor.h \
    newbufferdialog.h \
    scenemodel.h \
    scenetreewidget.h \
    colourspacesliderswidget.h \
    model.h \
    colourcomponentsliderwidget.h \
    nodeeditorwidget.h \
    tool.h \
    type.h \
    stroke.h \
    colourpalettewidget.h \
    paletteeditorwidget.h

FORMS    += \
    mainwindow.ui \
    dabeditorwidget.ui \
    strokeeditorwidget.ui \
    transformeditorwidget.ui \
    sessioneditorwidget.ui \
    newbufferdialog.ui \
    scenetreewidget.ui \
    colourspacesliderswidget.ui \
    nodeeditorwidget.ui \
    paletteeditorwidget.ui

RESOURCES += \
    thirdparty/stylesheets/qdarkstyle/style.qrc \
    thirdparty/stylesheets/darkorange/darkorange.qrc \
    thirdparty/shaders/thirdpartyshaders.qrc \
    shaders/shaders.qrc

DISTFILES += \
    LICENSE \
    README.md \
    TODO
