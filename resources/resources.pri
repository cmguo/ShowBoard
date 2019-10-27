HEADERS += \
    $$PWD/graph.h \
    $$PWD/resources.h \
    $$PWD/stroke.h \
    $$PWD/strokeparser.h

SOURCES += \
    $$PWD/graph.cpp \
    $$PWD/stroke.cpp \
    $$PWD/strokeparser.cpp

DISTFILES +=

include(graph/graph.pri)
