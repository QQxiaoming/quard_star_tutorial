TARGETPATH = QtQml
AUX_QML_FILES += plugins.qmltypes

load(qml_module)

# qmltypes target
!cross_compile:if(build_pass|!debug_and_release) {
    qtPrepareTool(QMLPLUGINDUMP, qmlplugindump)

    qmltypes.commands = $$QMLPLUGINDUMP -nonrelocatable -noforceqtquick QtQml 2.$$QT_MINOR_VERSION > $$PWD/plugins.qmltypes
    QMAKE_EXTRA_TARGETS += qmltypes
}
