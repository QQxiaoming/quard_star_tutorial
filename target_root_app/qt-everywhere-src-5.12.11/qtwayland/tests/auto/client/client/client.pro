include (../shared/shared.pri)

TARGET = tst_client
SOURCES += tst_client.cpp

check.commands = $(TESTRUNNER) $${PWD}/run-with-all-shells.sh $(TESTARGS)
