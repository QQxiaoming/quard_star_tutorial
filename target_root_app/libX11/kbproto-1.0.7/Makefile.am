SUBDIRS = specs

kbdir = $(includedir)/X11/extensions
kb_HEADERS = \
	XKBgeom.h \
	XKB.h \
	XKBproto.h \
	XKBsrv.h \
	XKBstr.h

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = kbproto.pc


MAINTAINERCLEANFILES = ChangeLog INSTALL

.PHONY: ChangeLog INSTALL

INSTALL:
	$(INSTALL_CMD)

ChangeLog:
	$(CHANGELOG_CMD)

dist-hook: ChangeLog INSTALL
