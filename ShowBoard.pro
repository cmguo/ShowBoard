QT += widgets network multimediawidgets webenginewidgets svg quick quickwidgets qml

win32 { QT += axcontainer }

TEMPLATE = lib
DEFINES += SHOWBOARD_LIBRARY

CONFIG += c++14

include(../config.pri)

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    showboard.cpp

HEADERS += \
    ShowBoard_global.h \
    showboard.h

RESOURCES += \
    res/ShowBoard.qrc


include(core/core.pri)
include(resources/resources.pri)
include(controls/controls.pri)
include(opengl/opengl.pri)
include(tools/tools.pri)
include(stroke/stroke.pri)
include(views/views.pri)

win32 {
    include(office/office.pri)
}

includes.files = $$PWD/*.h
includes.core.files = $$PWD/core/*.h
includes.views.files = $$PWD/views/*.h
includes.tools.files = $$PWD/tools/*.h
includes.resources.files = $$PWD/resources/*.h
win32 {
    includes.path = $$[QT_INSTALL_HEADERS]/ShowBoard
    includes.core.path = $$[QT_INSTALL_HEADERS]/ShowBoard/core
    includes.views.path = $$[QT_INSTALL_HEADERS]/ShowBoard/views
    includes.tools.path = $$[QT_INSTALL_HEADERS]/ShowBoard/tools
    includes.resources.path = $$[QT_INSTALL_HEADERS]/ShowBoard/resources
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

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../QtEventBus/release/ -lQtEventBus
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../QtEventBus/debug/ -lQtEventBusd
else:unix: LIBS += -L$$OUT_PWD/../QtEventBus/ -lQtEventBus

INCLUDEPATH += $$PWD/../QtEventBus
DEPENDPATH += $$PWD/../QtEventBus

INCLUDEPATH += $$PWD/../QtPromise/src

INCLUDEPATH += $$PWD/../qtpromise/src/qtpromise $$PWD/../qtpromise/include
#DEPENDPATH += $$PWD/../qtpromise/src/qtpromise

win32:CONFIG(debug, debug|release): {
    LIBS += -lGdiplus
}

