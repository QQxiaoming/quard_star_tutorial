AUTOMAKE_OPTIONS=dejagnu

DISTFILES=Makefile.am Makefile.in
CLEANFILES=lrzsz.log lrzsz.sum site.bak
DISTCLEANFILES=site.exp

dist-hook:
	mkdir $(distdir)/config
	mkdir $(distdir)/lib
	mkdir $(distdir)/lrzsz
	cp -p $(srcdir)/config/*.exp $(distdir)/config
	cp -p $(srcdir)/lib/*.exp $(distdir)/lib
	cp -p $(srcdir)/lrzsz/*.exp $(distdir)/lrzsz
