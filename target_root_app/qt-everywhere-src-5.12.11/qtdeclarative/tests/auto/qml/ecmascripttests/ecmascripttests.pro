TEMPLATE = subdirs
SUBDIRS = testcase.pro qjstest

checkjittarget.target = check-jit
checkjittarget.commands = qjstest --jit --parallel --with-test-expectations --update-expectations
checkjittarget.depends = all
QMAKE_EXTRA_TARGETS += checkjittarget

checkmothtarget.target = check-interpreter
checkmothtarget.commands = qjstest --interpret --parallel --with-test-expectations
checkmothtarget.depends = all
QMAKE_EXTRA_TARGETS += checkmothtarget

