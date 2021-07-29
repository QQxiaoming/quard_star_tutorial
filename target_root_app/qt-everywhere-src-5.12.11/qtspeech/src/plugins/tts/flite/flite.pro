TARGET = qttexttospeech_flite
QT = core multimedia texttospeech

PLUGIN_TYPE = texttospeech
PLUGIN_CLASS_NAME = QTextToSpeechEngineFlite
load(qt_plugin)

include(../common/common.pri)

HEADERS += \
    qtexttospeech_flite.h \
    qtexttospeech_flite_plugin.h \
    qtexttospeech_flite_processor.h

SOURCES += \
    qtexttospeech_flite.cpp \
    qtexttospeech_flite_plugin.cpp \
    qtexttospeech_flite_processor.cpp

OTHER_FILES += \
    flite_plugin.json

LIBS += -lflite_cmu_us_kal16 -lflite_usenglish -lflite_cmulex -lflite

config_flite_alsa: LIBS += -lasound
