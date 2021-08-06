## -*- makefile-automake -*-
## Copyright (C) 1995-2018 Free Software Foundation, Inc.
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2, or (at your option)
## any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program.  If not, see <https://www.gnu.org/licenses/>.

## ---------------- ##
##  Documentation.  ##
## ---------------- ##

info_TEXINFOS = %D%/automake.texi %D%/automake-history.texi
doc_automake_TEXINFOS = %D%/fdl.texi
doc_automake_history_TEXINFOS = %D%/fdl.texi

man1_MANS = \
  %D%/aclocal.1 \
  %D%/automake.1 \
  %D%/aclocal-$(APIVERSION).1 \
  %D%/automake-$(APIVERSION).1

$(man1_MANS): $(top_srcdir)/configure.ac

CLEANFILES += $(man1_MANS)
# XXX: This script should be updated with 'fetch' target.
EXTRA_DIST += %D%/help2man

update_mans = \
  $(AM_V_GEN): \
    && $(MKDIR_P) %D% \
    && ./pre-inst-env $(PERL) $(srcdir)/%D%/help2man --output=$@

%D%/aclocal.1 %D%/automake.1:
	$(AM_V_GEN): \
	  && $(MKDIR_P) %D% \
	  && f=`echo $@ | sed 's|.*/||; s|\.1$$||; $(transform)'` \
	  && echo ".so man1/$$f-$(APIVERSION).1" > $@

%D%/aclocal-$(APIVERSION).1: $(aclocal_script) lib/Automake/Config.pm
	$(update_mans) aclocal-$(APIVERSION)
%D%/automake-$(APIVERSION).1: $(automake_script) lib/Automake/Config.pm
	$(update_mans) automake-$(APIVERSION)

## ---------------------------- ##
##  Example package "amhello".  ##
## ---------------------------- ##

amhello_sources = \
  %D%/amhello/configure.ac \
  %D%/amhello/Makefile.am \
  %D%/amhello/README \
  %D%/amhello/src/main.c \
  %D%/amhello/src/Makefile.am

amhello_configury = \
  aclocal.m4 \
  autom4te.cache \
  Makefile.in \
  config.h.in \
  configure \
  depcomp \
  install-sh \
  missing \
  src/Makefile.in

dist_noinst_DATA += $(amhello_sources)
dist_doc_DATA = $(srcdir)/%D%/amhello-1.0.tar.gz

setup_autotools_paths = { \
  ACLOCAL=aclocal-$(APIVERSION) && export ACLOCAL \
    && AUTOMAKE=automake-$(APIVERSION) && export AUTOMAKE \
    && AUTOCONF='$(am_AUTOCONF)' && export AUTOCONF \
    && AUTOM4TE='$(am_AUTOM4TE)' && export AUTOM4TE \
    && AUTORECONF='$(am_AUTORECONF)' && export AUTORECONF \
    && AUTOHEADER='$(am_AUTOHEADER)' && export AUTOHEADER \
    && AUTOUPDATE='$(am_AUTOUPDATE)' && export AUTOUPDATE \
    && true; \
}

# We depend on configure.ac so that we regenerate the tarball
# whenever the Automake version changes.
$(srcdir)/%D%/amhello-1.0.tar.gz: $(amhello_sources) $(srcdir)/configure.ac
	$(AM_V_GEN)tmp=amhello-output.tmp \
	  && $(am__cd) $(srcdir)/%D%/amhello \
	  && : Make our	aclocal and automake avaiable before system ones. \
	  && $(setup_autotools_paths) \
	  && ( \
	    { $(AM_V_P) || exec 5>&2 >$$tmp 2>&1; } \
	      && $(abs_builddir)/pre-inst-env $(am_AUTORECONF) -vfi \
	      && ./configure \
	      && $(MAKE) $(AM_MAKEFLAGS) distcheck \
	      && $(MAKE) $(AM_MAKEFLAGS) distclean \
	      || { \
	        if $(AM_V_P); then :; else \
	          echo "$@: recipe failed." >&5; \
	          echo "See file '`pwd`/$$tmp' for details" >&5; \
		fi; \
	        exit 1; \
	      } \
	  ) \
	  && rm -rf $(amhello_configury) $$tmp \
	  && mv -f amhello-1.0.tar.gz ..


# vim: ft=automake noet
