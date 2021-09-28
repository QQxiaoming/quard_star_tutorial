bin_PROGRAMS=lrz lsz
lrz_SOURCES=lrz.c timing.c zperr.c zreadline.c crctab.c rbsb.c zm.c protname.c tcp.c lsyslog.c canit.c
lsz_SOURCES=lsz.c timing.c zperr.c zreadline.c crctab.c rbsb.c zm.c protname.c tcp.c lsyslog.c canit.c
noinst_HEADERS = timing.h zglobal.h zmodem.h
datadir = $(prefix)/@DATADIRNAME@
localedir = $(datadir)/locale
CFLAGS=@CFLAGS@
DISTCLEAN_FILES=lrzszbug

LDADD = ../lib/libzmodem.a @INTLLIBS@
AUTOMAKE_OPTIONS=ansi2knr
EXTRA_DIST = ansi2knr.1 ansi2knr.c lrzszbug.in
INCLUDES = -I.. -I$(srcdir) -I$(top_srcdir)/src -I../intl -I$(top_srcdir)/lib
#DEFS = -DLOCALEDIR=\"$(localedir)\" -DOS=\"@host_os@\" -DCPU=\"@host_cpu@\"
DEFS = -DNFGVMIN @DEFS@ -DLOCALEDIR=\"$(localedir)\"

install-exec-local:
	rm -f $(DESTDIR)/$(bindir)/`echo lsb | sed -e '$(transform)'`
	ln $(DESTDIR)/$(bindir)/`echo lsz |sed -e '$(transform)'` \
		$(DESTDIR)/$(bindir)/`echo lsb |sed -e '$(transform)'` 
	rm -f $(bindir)/`echo lsx | sed -e '$(transform)'`
	ln $(DESTDIR)/$(bindir)/`echo lsz |sed -e '$(transform)'` \
		$(DESTDIR)/$(bindir)/`echo lsx |sed -e '$(transform)'` 
	rm -f $(bindir)/`echo lrb | sed -e '$(transform)'`
	ln $(DESTDIR)/$(bindir)/`echo lrz |sed -e '$(transform)'` \
		$(DESTDIR)/$(bindir)/`echo lrb |sed -e '$(transform)'` 
	rm -f $(bindir)/`echo lrx | sed -e '$(transform)'`
	ln $(DESTDIR)/$(bindir)/`echo lrz |sed -e '$(transform)'` \
		$(DESTDIR)/$(bindir)/`echo lrx |sed -e '$(transform)'` 
#	rm -f $(bindir)/`echo lrzszbug | sed -e '$(transform)'`
#	ln $(bindir)/`echo lrzszbug |sed -e '$(transform)'` \
#		$(bindir)/`echo lrzszbug |sed -e '$(transform)'` 

uninstall-local:
	rm -f $(DESTDIR)/$(bindir)/`echo lsb |sed -e '$(transform)'` 
	rm -f $(DESTDIR)/$(bindir)/`echo lsx |sed -e '$(transform)'` 
	rm -f $(DESTDIR)/$(bindir)/`echo lrb |sed -e '$(transform)'` 
	rm -f $(DESTDIR)/$(bindir)/`echo lrx |sed -e '$(transform)'` 
#	rm -f $(bindir)/`echo lrzszbug |sed -e '$(transform)'` 
