TARGET = qtt9write_db
CONFIG += static

HEADERS += $$files(api/*)

T9WRITE_RESOURCE_FILES = \
    $$files(data/arabic/*.bin) \
    $$files(data/hebrew/*.bin) \
    $$files(data/thai/*.bin) \
    $$files(data/*.bin) \
    $$files(data/*.ldb) \
    $$files(data/*.hdb) \
    $$files(data/*.phd)

# Note: Compression is disabled, because the resource is accessed directly from the memory
QMAKE_RESOURCE_FLAGS += -no-compress
CONFIG += resources_big

t9write_db.files = $$T9WRITE_RESOURCE_FILES
t9write_db.prefix =/QtQuick/VirtualKeyboard/T9Write
RESOURCES += t9write_db

load(qt_helper_lib)

# Needed for resources
CONFIG += qt
QT = core
