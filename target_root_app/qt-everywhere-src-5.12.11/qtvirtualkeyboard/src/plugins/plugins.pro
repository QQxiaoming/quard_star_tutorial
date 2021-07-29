TEMPLATE = subdirs

include(../config.pri)

hangul: SUBDIRS += hangul
hunspell: SUBDIRS += hunspell
lipi-toolkit: {
    SUBDIRS += lipi-toolkit
    !disable-hunspell: lipi-toolkit.depends += hunspell
}
openwnn: SUBDIRS += openwnn
pinyin: SUBDIRS += pinyin
t9write: SUBDIRS += t9write
tcime: SUBDIRS += tcime
myscript: SUBDIRS += myscript
thai: {
    SUBDIRS += thai
    !disable-hunspell: thai.depends += hunspell
}
