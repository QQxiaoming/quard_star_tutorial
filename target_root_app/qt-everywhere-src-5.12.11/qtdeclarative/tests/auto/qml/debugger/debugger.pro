TEMPLATE = subdirs

SUBDIRS += qqmldebugjsserver

PUBLICTESTS += \
    qdebugmessageservice \
    qqmlenginedebugservice \
    qqmldebugjs \
    qqmlinspector \
    qqmlprofilerservice \
    qpacketprotocol \
    qqmlenginedebuginspectorintegrationtest \
    qqmlenginecontrol \
    qqmldebuggingenabler \
    qqmlnativeconnector \
    qqmldebugprocess \
    qqmlpreview

PRIVATETESTS += \
    qqmldebugclient \
    qqmldebuglocal \
    qqmldebugservice \
    qv4debugger

SUBDIRS += $$PUBLICTESTS

qqmldebugjs.depends = qqmldebugjsserver
qqmlprofilerservice.depends = qqmldebugjsserver

qtConfig(private_tests): \
    SUBDIRS += $$PRIVATETESTS

