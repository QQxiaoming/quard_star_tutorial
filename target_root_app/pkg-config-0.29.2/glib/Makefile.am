## Process this file with automake to produce Makefile.in
include $(top_srcdir)/glib.mk

# http://people.gnome.org/~walters/docs/build-api.txt
.buildapi-allow-builddir:

ACLOCAL_AMFLAGS = -I m4macros ${ACLOCAL_FLAGS}

SUBDIRS = . m4macros glib
DIST_SUBDIRS = $(SUBDIRS)

AM_CPPFLAGS = 					\
	-DG_LOG_DOMAIN=g_log_domain_glib 	\
	@GLIB_DEBUG_FLAGS@ 			\
	-DG_DISABLE_DEPRECATED 			\
	-DGLIB_COMPILATION

EXTRA_DIST += 			\
	acglib.m4		\
	README.in		\
	glib.mk			\
	$(NULL)


# These may be in the builddir too
BUILT_EXTRA_DIST += 		\
	README			\
	$(NULL)

CONFIGURE_DEPENDENCIES = acglib.m4

# build documentation when doing distcheck
DISTCHECK_CONFIGURE_FLAGS = --enable-debug --enable-gtk-doc --enable-man --disable-maintainer-mode

DISTCLEANFILES += config.lt

distclean-local: lcov-clean
	if test $(srcdir) = .; then :; else \
	    rm -f $(BUILT_EXTRA_DIST); \
	fi

.PHONY: files release sanity snapshot

files:
	@files=`ls $(DISTFILES) 2> /dev/null `; for p in $$files; do \
	  echo $$p; \
	done

release:
	rm -rf .deps */.deps
	cd docs && make glib.html
	$(MAKE) distcheck

sanity:
	./sanity_check $(VERSION)

snapshot:
	$(MAKE) dist distdir=$(PACKAGE)-snap`date +"%Y%m%d"`

snapcheck:
	$(MAKE) distcheck distdir=$(PACKAGE)-snap`date +"%Y%m%d"`

dist-hook: $(BUILT_EXTRA_DIST)
	files='$(BUILT_EXTRA_DIST)'; \
	for f in $$files; do \
	  if test -f $$f; then d=.; else d=$(srcdir); fi; \
	  rm -f $(distdir)/$$f && cp $$d/$$f $(distdir) || exit 1; done
