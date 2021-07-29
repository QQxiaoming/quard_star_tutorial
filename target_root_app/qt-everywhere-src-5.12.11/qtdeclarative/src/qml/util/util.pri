SOURCES += \
    $$PWD/qqmlchangeset.cpp \
    $$PWD/qqmllistaccessor.cpp \
    $$PWD/qqmllistcompositor.cpp \
    $$PWD/qqmlpropertymap.cpp

HEADERS += \
    $$PWD/qqmlchangeset_p.h \
    $$PWD/qqmllistaccessor_p.h \
    $$PWD/qqmllistcompositor_p.h \
    $$PWD/qqmlpropertymap.h

qtConfig(qml-delegate-model) {
    SOURCES += \
        $$PWD/qqmladaptormodel.cpp

    HEADERS += \
        $$PWD/qqmladaptormodel_p.h
}
