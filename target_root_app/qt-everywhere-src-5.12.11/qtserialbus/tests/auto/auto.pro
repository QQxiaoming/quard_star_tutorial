TEMPLATE = subdirs
SUBDIRS += cmake \
           qcanbusframe \
           qcanbusdevice \
           qmodbusdataunit \
           qmodbusreply \
           qmodbusdevice \
           qmodbuspdu \
           qmodbusclient \
           qmodbusserver \
           qmodbuscommevent \
           qmodbusadu \
           qmodbusdeviceidentification \
           qmodbusrtuserialmaster

!android: SUBDIRS += qcanbus

qcanbus.depends += plugins
qcanbusdevice.depends += plugins

SUBDIRS += plugins
