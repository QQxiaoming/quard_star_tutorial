TEMPLATE = subdirs
QT_FOR_CONFIG += qml-private

qtConfig(qml-devtools) {
    SUBDIRS += \
        qmllint \
        qmlmin \
        qmlimportscanner

    qtConfig(commandlineparser):qtConfig(xmlstreamwriter): SUBDIRS += qmlcachegen
}

qtConfig(thread):!android|android_app {
    SUBDIRS += \
        qml

    qtConfig(qml-profiler): SUBDIRS += qmlprofiler
    qtConfig(qml-preview): SUBDIRS += qmlpreview

    qtHaveModule(quick) {
        !static: {
            SUBDIRS += \
                qmlscene \
                qmltime

            qtConfig(regularexpression):qtConfig(process) {
                SUBDIRS += \
                    qmlplugindump
            }
        }
        qtHaveModule(widgets): SUBDIRS += qmleasing
    }
    qtHaveModule(qmltest): SUBDIRS += qmltestrunner
    qtConfig(private_tests): SUBDIRS += qmljs
}

qtConfig(qml-devtools) {
    qml.depends = qmlimportscanner
    qmleasing.depends = qmlimportscanner
}

# qmlmin, qmlimportscanner & qmlcachegen are build tools.
# qmlscene is needed by the autotests.
# qmltestrunner may be useful for manual testing.
# qmlplugindump cannot be a build tool, because it loads target plugins.
# The other apps are mostly "desktop" tools and are thus excluded.
qtNomakeTools( \
    qmlprofiler \
    qmlplugindump \
    qmleasing \
)
