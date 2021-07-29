TARGET = qtsensors_sensortag
QT = core sensors-private bluetooth
CONFIG += c++11

include(sensortag.pri)

OTHER_FILES = plugin.json

PLUGIN_TYPE = sensors
PLUGIN_CLASS_NAME = SensortagSensorPlugin
load(qt_plugin)
