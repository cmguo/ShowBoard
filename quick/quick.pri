HEADERS += \
    $$PWD/qquickshapeitem.h \
    $$PWD/qquickshapepath.h \
    $$PWD/quickhelper.h

SOURCES += \
    $$PWD/qquickshapeitem.cpp \
    $$PWD/qquickshapepath.cpp \
    $$PWD/quickhelper.cpp

showboard_quick : {

    HEADERS += \
        $$PWD/itemframe.h \
        $$PWD/positionbar.h \
        $$PWD/selectbox.h \
        $$PWD/stateitem.h

    SOURCES += \
        $$PWD/itemframe.cpp \
        $$PWD/positionbar.cpp \
        $$PWD/selectbox.cpp \
        $$PWD/stateitem.cpp

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
