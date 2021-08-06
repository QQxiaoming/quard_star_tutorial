ACLOCAL_AMFLAGS = ${ACLOCAL_FLAGS}

if INTERNAL_GLIB
GLIB_SUBDIR = glib
endif

# Normally we'd want glib to be part of DIST_SUBDIRS unconditionally,
# but distclean gets broken unless we always run glib's configure
SUBDIRS = $(GLIB_SUBDIR) . check
DIST_SUBDIRS = $(SUBDIRS)

# Escape paths properly on windows
if NATIVE_WIN32
AM_CPPFLAGS = \
	-DPKG_CONFIG_PC_PATH="\"$(subst /,\/,$(pc_path))\"" \
	-DPKG_CONFIG_SYSTEM_INCLUDE_PATH="\"$(subst /,\/,$(system_include_path))\"" \
	-DPKG_CONFIG_SYSTEM_LIBRARY_PATH="\"$(subst /,\/,$(system_library_path))\""
else
AM_CPPFLAGS = \
	-DPKG_CONFIG_PC_PATH="\"$(pc_path)\"" \
	-DPKG_CONFIG_SYSTEM_INCLUDE_PATH="\"$(system_include_path)\"" \
	-DPKG_CONFIG_SYSTEM_LIBRARY_PATH="\"$(system_library_path)\""
endif

AM_CFLAGS = \
	$(WARN_CFLAGS) \
	$(GCOV_CFLAGS) \
	$(GLIB_CFLAGS)

bin_PROGRAMS = pkg-config
pkg_config_LDADD = $(GLIB_LIBS)

include Makefile.sources

if HOST_TOOL
host_tool = $(host)-pkg-config$(EXEEXT)
install-exec-hook:
	cd $(DESTDIR)$(bindir) && $(LN) -f pkg-config$(EXEEXT) $(host_tool)
uninstall-hook:
	cd $(DESTDIR)$(bindir) && rm -f $(host_tool)
endif

# Various data files
m4dir = $(datadir)/aclocal
m4_DATA = pkg.m4
dist_doc_DATA = pkg-config-guide.html
dist_man_MANS = pkg-config.1
EXTRA_DIST =	\
	README.win32		\
	detectenv-msvc.mak	\
	Makefile.vc		\
	config.h.win32

# gcov test coverage
gcov:
	-$(MAKE) $(AM_MAKEFLAGS) -k check
	$(GCOV) $(pkg_config_SOURCES)
CLEANFILES = *.gcda *.gcno *.gcov

# Since we can't always have glib in DIST_SUBDIRS, we need to make sure
# glib is configured when we want to run dist. Unfortunately, there's no
# DIST_CONFIGURE_FLAGS.
DISTCHECK_CONFIGURE_FLAGS = --with-internal-glib
if !INTERNAL_GLIB
dist-hook:
	@echo "error: --with-internal-glib is required to include glib in dist"
	@exit 1
endif
