defineReplace(qtwebengine_extractCFlag) {
    CFLAGS = $$QMAKE_CC $$QMAKE_CFLAGS
    OPTION = $$find(CFLAGS, $$1)
    OPTION = $$split(OPTION, =)
    PARAM = $$member(OPTION, 1)
    !isEmpty(PARAM): return ($$PARAM)
    return ($$OPTION)
}
