QT += widgets network \
    svg quick quickwidgets qml concurrent websockets

TEMPLATE = lib
DEFINES += SHOWBOARD_LIBRARY

CONFIG += c++14

include($$(applyCommonConfig))
include($$(applyConanPlugin))

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

SHOWBOARD_RECORD=$$(SHOWBOARD_RECORD)
equals(SHOWBOARD_RECORD,): SHOWBOARD_RECORD=0
DEFINES += SHOWBOARD_RECORD=$${SHOWBOARD_RECORD}

DEFINES += SHOWBOARD_RECORD_PER_PAGE=0

include(core/core.pri)
include(resources/resources.pri)
include(controls/controls.pri)
include(tools/tools.pri)
include(stroke/stroke.pri)
include(views/views.pri)
include(data/data.pri)
include(graphics/graphics.pri)
include(widget/widget.pri)
include(quick/quick.pri)

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
