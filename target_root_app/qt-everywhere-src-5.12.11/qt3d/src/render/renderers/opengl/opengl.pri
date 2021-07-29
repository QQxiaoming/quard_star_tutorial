
include (renderer/renderer.pri)
include (jobs/jobs.pri)
include (io/io.pri)
include (textures/textures.pri)
include (graphicshelpers/graphicshelpers.pri)
include (renderstates/renderstates.pri)

# Qt3D is free of Q_FOREACH - make sure it stays that way:
DEFINES += QT_NO_FOREACH

gcov {
    QMAKE_CXXFLAGS += -fprofile-arcs -ftest-coverage
    QMAKE_LFLAGS += -fprofile-arcs -ftest-coverage
}
