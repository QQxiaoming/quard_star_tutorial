option(host_build)

gcc:equals(QT_ARCH, "x86_64"):contains(QT_TARGET_ARCH, "arm"):!contains(QT_TARGET_ARCH, "arm64") {
    QMAKE_CXXFLAGS += -m32
    QMAKE_LFLAGS += -m32
}

SOURCES = main.cpp

