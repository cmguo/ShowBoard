HEADERS += \
    $$PWD/controls.h \
    $$PWD/imagecontrol.h \
    $$PWD/qmlcontrol.h \
    $$PWD/strokecontrol.h \
    $$PWD/textcontrol.h \
    $$PWD/unknowncontrol.h \
    $$PWD/videocontrol.h \
    $$PWD/webcontrol.h \
    $$PWD/whitecanvascontrol.h \
    $$PWD/widgetcontrol.h

SOURCES += \
    $$PWD/imagecontrol.cpp \
    $$PWD/qmlcontrol.cpp \
    $$PWD/strokecontrol.cpp \
    $$PWD/textcontrol.cpp \
    $$PWD/unknowncontrol.cpp \
    $$PWD/videocontrol.cpp \
    $$PWD/webcontrol.cpp \
    $$PWD/whitecanvascontrol.cpp \
    $$PWD/widgetcontrol.cpp

win32 {
    HEADERS += \
        $$PWD/pptxcontrol.h \
        $$PWD/wordcontrol.h

    SOURCES += \
        $$PWD/pptxcontrol.cpp \
        $$PWD/wordcontrol.cpp
}
