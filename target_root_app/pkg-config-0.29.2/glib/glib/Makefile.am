## Process this file with automake to produce Makefile.in
include $(top_srcdir)/glib.mk

#
# Generate glibconfig.h
#
# The timestamp of the stamp file is used to indicate if glibconfig.h is
# up to date with respect to config.status.  In the usual case, changes
# to config.status will not result in changes to glibconfig.h so we
# avoid touching its timestamp (in order not to rebuild the whole tree).
#
DISTCLEANFILES += glibconfig-stamp glibconfig.h
BUILT_SOURCES += glibconfig-stamp
configexecincludedir = $(libdir)/glib-2.0/include
nodist_noinst_HEADERS = glibconfig.h
glibconfig-stamp: ../config.status
	$(AM_V_GEN) cd $(top_builddir) && \
	  $(SHELL) ./config.status glib/glibconfig.h
	@touch glibconfig-stamp



if HAVE_GOOD_PRINTF
else
PRINTF_SUBDIR = gnulib
printf_la = gnulib/libgnulib.la
endif 

SUBDIRS = libcharset $(PRINTF_SUBDIR) .
DIST_SUBDIRS = libcharset gnulib

AM_CPPFLAGS = 				\
	$(glib_INCLUDES) 		\
	$(pcre_inc) 			\
	-DG_LOG_DOMAIN=\"GLib\" 	\
	$(GLIB_DEBUG_FLAGS) 		\
	-DGLIB_COMPILATION 		\
	-DPCRE_STATIC

AM_CFLAGS = $(GLIB_WARN_CFLAGS)

MIRRORING_TAB_SOURCE = 				\
	glib-mirroring-tab/Makefile 		\
	glib-mirroring-tab/gen-mirroring-tab.c	\
	glib-mirroring-tab/packtab.h		\
	glib-mirroring-tab/packtab.c

EXTRA_DIST += 			\
	glib.rc.in		\
	gen-iswide-table.py	\
	gen-unicode-tables.pl	\
	gen-script-table.pl	\
	win_iconv.c		\
	docs.c			\
	gconstructor.h		\
	$(MIRRORING_TAB_SOURCE)

# These may be in the builddir too
BUILT_EXTRA_DIST += 		\
	glib.rc

noinst_LTLIBRARIES += libglib-2.0.la

if OS_WIN32_AND_DLL_COMPILATION
if MS_LIB_AVAILABLE
noinst_DATA += glib-2.0.lib

install_ms_lib_cmd = $(INSTALL) glib-2.0.lib $(DESTDIR)$(libdir)
uninstall_ms_lib_cmd = -rm $(DESTDIR)$(libdir)/glib-2.0.lib
endif
endif

install-ms-lib:
	$(install_ms_lib_cmd)

uninstall-ms-lib:
	$(uninstall_ms_lib_cmd)

deprecated_sources = \
	deprecated/gallocator.c		\
	deprecated/gcache.c		\
	deprecated/gcompletion.c	\
	deprecated/grel.c		\
	deprecated/gthread-deprecated.c

libglib_2_0_la_SOURCES = 	\
	$(deprecated_sources)	\
	garray.c		\
	gasyncqueue.c		\
	gasyncqueueprivate.h	\
	gatomic.c		\
	gbacktrace.c		\
	gbase64.c		\
	gbitlock.c		\
	gbookmarkfile.c 	\
	gbsearcharray.h		\
	gbytes.c		\
	gbytes.h		\
	gcharset.c		\
	gcharsetprivate.h	\
	gchecksum.c		\
	gconvert.c		\
	gdataset.c		\
	gdatasetprivate.h	\
	gdate.c	 		\
	gdatetime.c	 	\
	gdir.c			\
	genviron.c		\
	gerror.c		\
	gfileutils.c		\
	ggettext.c		\
	ghash.c			\
	ghmac.c			\
	ghook.c			\
	ghostutils.c		\
	giochannel.c    	\
	gkeyfile.c		\
	glibintl.h		\
	glib_trace.h		\
	glib-init.h		\
	glib-init.c		\
	glib-private.h		\
	glib-private.c		\
	glist.c			\
	gmain-internal.h	\
	gmain.c	 		\
	gmappedfile.c		\
	gmarkup.c		\
	gmem.c			\
	gmessages.c		\
	gmessages-private.h	\
	gmirroringtable.h	\
	gnode.c			\
	goption.c		\
	gpattern.c		\
	gpoll.c			\
	gprimes.c		\
	gqsort.c		\
	gquark.c		\
	gqueue.c		\
	grand.c			\
	gscanner.c		\
	gscripttable.h		\
	gsequence.c		\
	gshell.c		\
	gslice.c		\
	gslist.c		\
	gstdio.c		\
	gstrfuncs.c		\
	gstring.c		\
	gstringchunk.c		\
	gtestutils.c		\
	gthread.c		\
	gthreadprivate.h	\
	gthreadpool.c		\
	gtimer.c		\
	gtimezone.c	 	\
	gtrashstack.c		\
	gtree.c			\
	guniprop.c		\
	gutf8.c			\
	gunibreak.h		\
	gunibreak.c		\
	gunichartables.h	\
	gunicollate.c		\
	gunicomp.h		\
	gunidecomp.h		\
	gunidecomp.c		\
	gunicodeprivate.h	\
	gurifuncs.c 		\
	gutils.c		\
	gvariant.h		\
	gvariant.c		\
	gvariant-core.h		\
	gvariant-core.c		\
	gvariant-internal.h	\
	gvariant-parser.c	\
	gvariant-serialiser.h	\
	gvariant-serialiser.c	\
	gvarianttypeinfo.h	\
	gvarianttypeinfo.c	\
	gvarianttype.c		\
	gversion.c		\
	gwakeup.h		\
	gwakeup.c		\
	gprintf.c		\
	gprintfint.h		\
	valgrind.h

if OS_UNIX
libglib_2_0_la_SOURCES += glib-unix.c
endif

if THREADS_WIN32
libglib_2_0_la_SOURCES += gthread-win32.c
else
if THREADS_POSIX
libglib_2_0_la_SOURCES += gthread-posix.c
endif
endif

EXTRA_libglib_2_0_la_SOURCES = \
	giounix.c	\
	giowin32.c	\
	gspawn.c	\
	gspawn-win32.c	\
	gwin32.c

glibincludedir=$(includedir)/glib-2.0
noinst_HEADERS =   \
	glib-unix.h     \
	glib-object.h	\
	glib.h

deprecatedincludedir=$(includedir)/glib-2.0/glib/deprecated
noinst_HEADERS += \
	deprecated/gallocator.h		\
	deprecated/gcache.h		\
	deprecated/gcompletion.h	\
	deprecated/gmain.h		\
	deprecated/grel.h		\
	deprecated/gthread.h

glibsubincludedir=$(includedir)/glib-2.0/glib
noinst_HEADERS += \
	galloca.h	\
	garray.h	\
	gasyncqueue.h	\
	gatomic.h	\
	gbacktrace.h	\
	gbase64.h	\
	gbitlock.h	\
	gbookmarkfile.h	\
	gbytes.h	\
	gcharset.h	\
	gchecksum.h	\
	gconvert.h	\
	gdataset.h	\
	gdate.h		\
	gdatetime.h	\
	gdir.h		\
	genviron.h	\
	gerror.h	\
	gfileutils.h	\
	ggettext.h	\
	ghash.h		\
	ghmac.h		\
	ghook.h		\
	ghostutils.h	\
	gi18n.h		\
	gi18n-lib.h	\
	giochannel.h	\
	gkeyfile.h 	\
	glist.h		\
	gmacros.h	\
	gmain.h		\
	gmappedfile.h	\
	gmarkup.h	\
	gmem.h		\
	gmessages.h	\
	gnode.h		\
	goption.h	\
	gpattern.h	\
	gpoll.h		\
	gprimes.h	\
	gqsort.h	\
	gquark.h	\
	gqueue.h	\
	grand.h		\
	gscanner.h	\
	gsequence.h	\
	gshell.h	\
	gslice.h	\
	gslist.h	\
	gspawn.h	\
	gstdio.h	\
	gstrfuncs.h	\
	gtestutils.h	\
	gstring.h	\
	gstringchunk.h	\
	gthread.h	\
	gthreadpool.h	\
	gtimer.h	\
	gtimezone.h	\
	gtrashstack.h	\
	gtree.h		\
	gtypes.h	\
	gunicode.h	\
	gurifuncs.h 	\
	gutils.h	\
	gvarianttype.h	\
	gvariant.h	\
	gversion.h	\
	gversionmacros.h \
	gwin32.h	\
	gprintf.h

# This is read by gobject-introspection/misc/ and gtk-doc
glib-public-headers.txt: Makefile
	echo $(glibinclude_HEADERS) $(glibsubinclude_HEADERS) > $@.tmp && mv $@.tmp $@

CLEANFILES += glib-public-headers.txt

all-local: glib-public-headers.txt

if PLATFORM_WIN32
no_undefined = -no-undefined
endif

if OS_WIN32_AND_DLL_COMPILATION
glib_win32_res = glib-win32-res.o
glib_win32_res_ldflag = -Wl,$(glib_win32_res)
endif

libglib_2_0_la_CFLAGS = $(AM_CFLAGS) $(GLIB_HIDDEN_VISIBILITY_CFLAGS)
libglib_2_0_la_LIBADD = libcharset/libcharset.la $(printf_la) @GIO@ @GSPAWN@ @PLATFORMDEP@ @ICONV_LIBS@ @G_LIBS_EXTRA@ $(pcre_lib) $(G_THREAD_LIBS_EXTRA) $(G_THREAD_LIBS_FOR_GTHREAD)
libglib_2_0_la_DEPENDENCIES = libcharset/libcharset.la $(printf_la) @GIO@ @GSPAWN@ @PLATFORMDEP@ $(glib_win32_res) $(glib_def)

libglib_2_0_la_LDFLAGS = $(GLIB_LINK_FLAGS) \
	 $(glib_win32_res_ldflag) \
	-export-dynamic $(no_undefined)

gspawn-win32-helper-console.c:
	echo '#define HELPER_CONSOLE' >$@
	echo '#include "gspawn-win32-helper.c"' >>$@

gspawn-win64-helper.c:
	echo '#include "gspawn-win32-helper.c"' >$@

gspawn-win64-helper-console.c:
	echo '#define HELPER_CONSOLE' >$@
	echo '#include "gspawn-win32-helper.c"' >>$@


glib-win32-res.o: glib.rc
	$(WINDRES) glib.rc $@

if OS_UNIX

auto_config_binscripts = gtester-report
EXTRA_DIST += ${auto_config_binscripts}

CONFIGVARS = \
	"bindir"    	: "${bindir}",		\
	"glib-version"  : "${GLIB_VERSION}"
endif

glib-2.0.lib: libglib-2.0.la glib.def
	lib -machine:@LIB_EXE_MACHINE_FLAG@ -name:libglib-2.0-$(LT_CURRENT_MINUS_AGE).dll -def:$(builddir)/glib.def -out:$@

dist-hook: $(BUILT_EXTRA_DIST)
	files='$(BUILT_EXTRA_DIST)'; \
	for f in $$files; do \
	  if test -f $$f; then d=.; else d=$(srcdir); fi; \
	  cp $$d/$$f $(distdir) || exit 1; done

../build/win32/vs9/glib.vcproj: $(top_srcdir)/build/win32/vs9/glib.vcprojin
	for F in `echo $(libglib_2_0_la_SOURCES) | tr '/' '\\'`; do \
		case $$F in \
		*-unix.c|gthread-*.c) \
			;; \
		*.c)	echo '   <File RelativePath="..\..\..\glib\'$$F'" />' \
			;; \
		esac; \
	done >libglib.sourcefiles
	$(CPP) -P - <$(top_srcdir)/build/win32/vs9/glib.vcprojin >$@
	rm libglib.sourcefiles

../build/win32/vs10/glib.vcxproj: $(top_srcdir)/build/win32/vs10/glib.vcxprojin
	for F in `echo $(libglib_2_0_la_SOURCES) | tr '/' '\\'`; do \
		case $$F in \
		*-unix.c|gthread-*.c) \
			;; \
		*.c) echo '    <ClCompile Include="..\..\..\glib\'$$F'" />' \
		     ;; \
		esac; \
	done >libglib.vs10.sourcefiles
	$(CPP) -P - <$(top_srcdir)/build/win32/vs10/glib.vcxprojin >$@
	rm libglib.vs10.sourcefiles

../build/win32/vs10/glib.vcxproj.filters: $(top_srcdir)/build/win32/vs10/glib.vcxproj.filtersin
	for F in `echo $(libglib_2_0_la_SOURCES) | tr '/' '\\'`; do \
		case $$F in \
		*-unix.c|gthread-*.c) \
			;; \
		*.c) echo '    <ClCompile Include="..\..\..\glib\'$$F'"><Filter>Source Files</Filter></ClCompile>' \
		     ;; \
		esac; \
	done >libglib.vs10.sourcefiles.filters
	$(CPP) -P - <$(top_srcdir)/build/win32/vs10/glib.vcxproj.filtersin >$@	
	rm libglib.vs10.sourcefiles.filters
