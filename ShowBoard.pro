QT += widgets network multimediawidgets webenginewidgets axcontainer svg quick quickwidgets qml

TEMPLATE = lib
DEFINES += SHOWBOARD_LIBRARY

CONFIG += c++11

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

QMAKE_CXXFLAGS += /utf-8

SOURCES += \

HEADERS += \
    ShowBoard_global.h \

RESOURCES += \
    ShowBoard.qrc


include(core/core.pri)
include(resources/resources.pri)
include(controls/controls.pri)
include(opengl/opengl.pri)
include(tools/tools.pri)
include(views/views.pri)
include(office/office.pri)

CONFIG(debug, debug|release) {
    win32: TARGET = $$join(TARGET,,,d)
}

msvc:CONFIG(release, debug|release) {
    QMAKE_CXXFLAGS+=/Zi
    QMAKE_LFLAGS+= /INCREMENTAL:NO /Debug
    target2.files = $$OUT_PWD/release/ShowBoard.pdb
    target2.path = $$[QT_INSTALL_LIBS]
    INSTALLS += target2
}

includes.files = $$PWD/*.h
includes.core.files = $$PWD/core/*.h
includes.views.files = $$PWD/views/*.h
win32 {
    includes.path = $$[QT_INSTALL_HEADERS]/ShowBoard
    includes.core.path = $$[QT_INSTALL_HEADERS]/ShowBoard/core
    includes.views.path = $$[QT_INSTALL_HEADERS]/ShowBoard/views
    target.path = $$[QT_INSTALL_LIBS]
}
INSTALLS += includes includes.core includes.views

# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../QtComposition/release/ -lQtComposition
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../QtComposition/debug/ -lQtCompositiond
else:unix: LIBS += -L$$OUT_PWD/../QtComposition/ -lQtComposition

INCLUDEPATH += $$PWD/../QtComposition
DEPENDPATH += $$PWD/../QtComposition

INCLUDEPATH += $$PWD/../QtPromise/src

INCLUDEPATH += $$PWD/../qtpromise/src/qtpromise $$PWD/../qtpromise/include
#DEPENDPATH += $$PWD/../qtpromise/src/qtpromise

win32 {
    LIBS += -lGdiplus
}
