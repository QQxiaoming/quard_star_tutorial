## Process this file with automake to produce Makefile.in
include $(top_srcdir)/glib.mk

AM_CPPFLAGS = 				\
	-DLIBDIR=\"$(libdir)\" 		\
	$(config_h_INCLUDES)

noinst_LTLIBRARIES += libcharset.la

libcharset_la_CFLAGS = $(GLIB_HIDDEN_VISIBILITY_CFLAGS)
libcharset_la_SOURCES =    	\
	libcharset.h		\
	localcharset.h		\
	localcharset.c

EXTRA_DIST += 			\
	README			\
	config.charset		\
	ref-add.sin		\
	ref-del.sin		\
	glibc21.m4		\
	codeset.m4		\
	update.sh		\
	make-patch.sh		\
	libcharset-glib.patch

charset_alias = $(DESTDIR)$(libdir)/charset.alias
charset_tmp = $(DESTDIR)$(libdir)/charset.tmp

charset.alias: config.charset
	$(AM_V_GEN) $(SHELL) $(srcdir)/config.charset '@host@' > t-$@
	@mv t-$@ $@

all-local: ref-add.sed ref-del.sed charset.alias

SUFFIXES = .sed .sin
.sin.sed:
	$(AM_V_GEN) $(SED) -e '/^#/d' -e 's/@''PACKAGE''@/@PACKAGE@/g' $< > t-$@
	@mv t-$@ $@

CLEANFILES += charset.alias ref-add.sed ref-del.sed
