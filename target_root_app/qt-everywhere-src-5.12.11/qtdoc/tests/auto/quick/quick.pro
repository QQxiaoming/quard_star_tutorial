TEMPLATE = subdirs

!cross_compile: PRIVATETESTS += examples

qtConfig(private_tests) {
    SUBDIRS += $$PRIVATETESTS
}
