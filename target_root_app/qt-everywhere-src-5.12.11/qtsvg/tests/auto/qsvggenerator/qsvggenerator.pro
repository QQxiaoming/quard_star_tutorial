TARGET = tst_qsvggenerator
CONFIG += testcase
QT += svg xml testlib widgets gui-private

SOURCES += tst_qsvggenerator.cpp

RESOURCES += referenceSvgs/fileName_output.svg \
             referenceSvgs/radial_gradient.svg
