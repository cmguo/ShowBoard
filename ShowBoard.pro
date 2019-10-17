QT += widgets network multimediawidgets webenginewidgets

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

SOURCES += \
    control.cpp \
    controlmanager.cpp \
    itemselector.cpp \
    resource.cpp \
    resourcemanager.cpp \
    resourceview.cpp \
    selectbox.cpp \
    toolbarwidget.cpp \
    whitecanvas.cpp \
    whitepage.cpp \
    widgetcontrol.cpp

HEADERS += \
    ShowBoard_global.h \
    control.h \
    controlmanager.h \
    itemselector.h \
    resource.h \
    resourcemanager.h \
    resourceview.h \
    selectbox.h \
    toolbarwidget.h \
    whitecanvas.h \
    whitepage.h \
    widgetcontrol.h

include(resources/resources.pri)
include(controls/controls.pri)
include(opengl/opengl.pri)

# Default rules for deployment.
unix {
    target.path = /usr/lib
}
!isEmpty(target.path): INSTALLS += target

win32:CONFIG(release, debug|release): LIBS += -L$$OUT_PWD/../QtComposition/release/ -lQtComposition
else:win32:CONFIG(debug, debug|release): LIBS += -L$$OUT_PWD/../QtComposition/debug/ -lQtComposition
else:unix: LIBS += -L$$OUT_PWD/../QtComposition/ -lQtComposition

INCLUDEPATH += $$PWD/../QtComposition
DEPENDPATH += $$PWD/../QtComposition

INCLUDEPATH += $$PWD/../QtPromise/src

INCLUDEPATH += $$PWD/../qtpromise/src/qtpromise $$PWD/../qtpromise/include
DEPENDPATH += $$PWD/../qtpromise/src/qtpromise
