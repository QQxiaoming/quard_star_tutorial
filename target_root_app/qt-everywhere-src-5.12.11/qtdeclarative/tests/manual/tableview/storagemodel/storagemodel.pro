TEMPLATE = app
TARGET = tableview_storage
QT += qml quick
SOURCES += main.cpp storagemodel.cpp
HEADERS += storagemodel.h
RESOURCES += main.qml

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
