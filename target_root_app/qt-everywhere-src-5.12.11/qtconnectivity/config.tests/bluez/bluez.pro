SOURCES += main.cpp

# test checks existence of certain bluez defines whose
# usage is known to violate the permissive test
QMAKE_CXXFLAGS += -fpermissive
