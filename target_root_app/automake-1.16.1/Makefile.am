## Process this file with automake to create Makefile.in

## Makefile for Automake.

# Copyright (C) 1995-2018 Free Software Foundation, Inc.

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

## Might be updated later.
CLEANFILES =
DISTCLEANFILES =
MAINTAINERCLEANFILES =
EXTRA_DIST =
TAGS_FILES =
dist_noinst_DATA =
nodist_noinst_DATA =
dist_noinst_SCRIPTS =
nodist_noinst_SCRIPTS =

## ------------ ##
##  Top level.  ##
## ------------ ##

EXTRA_DIST += \
  bootstrap \
  GNUmakefile \
  HACKING \
  PLANS

# We want a handful of substitutions to be fully-expanded by make;
# then use config.status to substitute the remainder where a single
# expansion is sufficient.  We use a funny notation here to avoid
# configure substitutions in our text.
do_subst = ( sed \
  -e "s,[@]configure_input[@],Generated from $$in; do not edit by hand.,g" \
  -e 's,[@]datadir[@],$(datadir),g' \
  -e 's,[@]amdir[@],$(amdir),g' \
  -e 's,[@]bindir[@],$(bindir),g' \
  -e 's,[@]docdir[@],$(docdir),g' \
  -e 's,[@]pkgvdatadir[@],$(pkgvdatadir),g' \
  -e 's,[@]scriptdir[@],$(scriptdir),g' \
  -e 's,[@]automake_acdir[@],$(automake_acdir),g' \
  -e 's,[@]system_acdir[@],$(system_acdir),g' \
## Hack to avoid a spurious substitution in the Automake script (part 1).
  -e 's,[@]am__isrc[@],!!@!!am__isrc!!@!!,g' \
  | $(SHELL) ./config.status --file=- \
## Hack to avoid a spurious substitution in the Automake script (part 2).
  | sed -e 's,!!@!!am__isrc!!@!!,@''am__isrc@,g' \
  )

# Generated  files shouldn't contain unexpanded '@substitutions@', and
# should be made read-only, to prevent them from being edited by mistake
# instead of the file the are generated from.
generated_file_finalize = $(AM_V_at) \
  if LC_ALL=C grep '@[a-zA-Z0-9_][a-zA-Z0-9_]*@' $@-t; then \
    echo "$@ contains unexpanded substitution (see lines above)"; \
    exit 1; \
  fi; \
  chmod a-w $@-t && mv -f $@-t $@

# Wrapper for the build environment.
nodist_noinst_SCRIPTS += pre-inst-env
CLEANFILES += $(noinst_SCRIPTS)

# The master location for INSTALL is lib/INSTALL.
# This is where "make fetch" will install new versions.
# Make sure we also update this copy.
INSTALL: lib/INSTALL
	$(AM_V_GEN)cp $(srcdir)/lib/INSTALL $@

# We don't use the default name for the autom4te cache directory,
# so we need this.
maintainer-clean-local:
	rm -rf .autom4te.cache

# So that automake won't complain about the missing ChangeLog.
# The real rule for ChangeLog generation is now in maintainer/maint.mk
# (as it is maintainer-specific).
ChangeLog:

# Third-party, obsolescent or experimental stuff.
EXTRA_DIST += \
  contrib/tap-driver.pl \
  contrib/check-html.am \
  contrib/multilib/README \
  contrib/multilib/config-ml.in \
  contrib/multilib/symlink-tree \
  contrib/multilib/multilib.am \
  contrib/multilib/multi.m4 \
  contrib/README

# Older files, kept mostly for historical interest.
EXTRA_DIST += \
  old/ChangeLog-tests \
  old/ChangeLog.96 \
  old/ChangeLog.98 \
  old/ChangeLog.00 \
  old/ChangeLog.01 \
  old/ChangeLog.02 \
  old/ChangeLog.03 \
  old/ChangeLog.04 \
  old/ChangeLog.09 \
  old/ChangeLog.11 \
  old/TODO

# Maintainer-specific files and scripts.
EXTRA_DIST += \
  maintainer/am-ft \
  maintainer/am-xft \
  maintainer/rename-tests \
  maintainer/maint.mk \
  maintainer/syntax-checks.mk

# Most work delegated to sub-dir makefile fragments.
include $(srcdir)/bin/local.mk
include $(srcdir)/doc/local.mk
include $(srcdir)/lib/local.mk
include $(srcdir)/lib/Automake/local.mk
include $(srcdir)/lib/am/local.mk
include $(srcdir)/m4/local.mk
include $(srcdir)/t/local.mk

# vim: ft=automake noet
