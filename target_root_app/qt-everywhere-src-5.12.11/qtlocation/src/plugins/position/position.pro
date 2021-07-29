TEMPLATE = subdirs

QT_FOR_CONFIG += positioning-private

linux|freebsd|openbsd|netbsd:qtHaveModule(dbus):SUBDIRS += geoclue
linux|freebsd|openbsd|netbsd:qtHaveModule(dbus):SUBDIRS += geoclue2
qtConfig(gypsy):SUBDIRS += gypsy
qtConfig(winrt_geolocation):SUBDIRS += winrt
qtHaveModule(simulator):SUBDIRS += simulator
osx|ios|tvos:SUBDIRS += corelocation
android:SUBDIRS += android
qtHaveModule(serialport):SUBDIRS += serialnmea

SUBDIRS += \
    positionpoll
