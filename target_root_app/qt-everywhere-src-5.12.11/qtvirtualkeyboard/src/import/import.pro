TARGET = qtquickvirtualkeyboardplugin
TARGETPATH = QtQuick/VirtualKeyboard
IMPORT_VERSION = 2.4
QT += qml quick virtualkeyboard-private

CONFIG += no_cxx_module

SOURCES += \
    qtquickvirtualkeyboardplugin.cpp

HEADERS += \
    qtquickvirtualkeyboardplugin.h

OTHER_FILES += \
    dependencies.json \
    plugins.qmltypes \
    qmldir

win32 {
    QMAKE_TARGET_PRODUCT = "Qt Virtual Keyboard (Qt $$QT_VERSION)"
    QMAKE_TARGET_DESCRIPTION = "Virtual Keyboard for Qt."
}

load(qml_plugin)

# qmltypes target override (requires QT_IM_MODULE=qtvirtualkeyboard, -defaultplatform and
# $$PWD/dependencies.json)
!cross_compile:if(build_pass|!debug_and_release) {
    qtPrepareTool(QMLPLUGINDUMP, qmlplugindump)
    # Note for win32 you need to set QT_IM_MODULE=qtvirtualkeyboard environment variable manually
    !win32: QMPPLUGINDUMP_ENV = "QT_IM_MODULE=qtvirtualkeyboard"
    qmltypes.commands = $$QMPPLUGINDUMP_ENV $$QMLPLUGINDUMP -defaultplatform -nonrelocatable -dependencies $$PWD/dependencies.json QtQuick.VirtualKeyboard $$IMPORT_VERSION > $$PWD/plugins.qmltypes
    QMAKE_EXTRA_TARGETS += qmltypes
}
