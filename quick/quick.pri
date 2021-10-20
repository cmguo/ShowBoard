HEADERS += \
    $$PWD/qquickshapeitem.h \
    $$PWD/qquickshapepath.h \
    $$PWD/quickhelper.h

SOURCES += \
    $$PWD/qquickshapeitem.cpp \
    $$PWD/qquickshapepath.cpp \
    $$PWD/quickhelper.cpp

showboard_quick {

    HEADERS += \
        $$PWD/itemframeq.h \
        $$PWD/positionbarq.h \
    $$PWD/selectboxq.h \
    $$PWD/stateitemq.h

    SOURCES += \
        $$PWD/itemframeq.cpp \
        $$PWD/positionbarq.cpp \
    $$PWD/selectboxq.cpp \
    $$PWD/stateitemq.cpp

} else {

    HEADERS += \
        $$PWD/quickproxyitem.h \
        $$PWD/quickwidgetitem.h \
        $$PWD/whitecanvasquick.h

    SOURCES += \
        $$PWD/quickproxyitem.cpp \
        $$PWD/quickwidgetitem.cpp \
        $$PWD/whitecanvasquick.cpp

}
