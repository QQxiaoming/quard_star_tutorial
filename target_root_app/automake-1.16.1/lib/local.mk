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

## -------------------------------------------------------------------- ##
##  Auxiliary scripts and files for use with "automake --add-missing".  ##
## -------------------------------------------------------------------- ##

dist_pkgvdata_DATA = \
  %D%/COPYING \
  %D%/INSTALL \
  %D%/texinfo.tex

# These must all be executable when installed.  However, if we use
# _SCRIPTS, then the program transform will be applied, which is not
# what we want.  So we make them executable by hand.
dist_script_DATA = \
  %D%/config.guess \
  %D%/config.sub \
  %D%/install-sh \
  %D%/mdate-sh \
  %D%/missing \
  %D%/mkinstalldirs \
  %D%/ylwrap \
  %D%/depcomp \
  %D%/compile \
  %D%/py-compile \
  %D%/ar-lib \
  %D%/test-driver \
  %D%/tap-driver.sh

install-data-hook:
	@$(POST_INSTALL)
	@for f in $(dist_script_DATA); do echo $$f; done \
	  | sed 's,^%D%/,,' \
	  | ( st=0; \
	      while read f; do \
	        echo " chmod +x '$(DESTDIR)$(scriptdir)/$$f'"; \
	        chmod +x "$(DESTDIR)$(scriptdir)/$$f" || st=1; \
	      done; \
	      exit $$st )

installcheck-local: installcheck-executable-scripts
installcheck-executable-scripts:
	@for f in $(dist_script_DATA); do echo $$f; done \
	  | sed 's,^%D%/,,' \
	  | while read f; do \
	      path="$(pkgvdatadir)/$$f"; \
	      test -x "$$path" || echo $$path; \
	    done \
	  | sed 's/$$/: not executable/' \
	  | grep . 1>&2 && exit 1; exit 0

# vim: ft=automake noet
