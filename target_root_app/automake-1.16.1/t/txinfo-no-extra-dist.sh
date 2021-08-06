#! /bin/sh
# Copyright (C) 1996-2018 Free Software Foundation, Inc.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

# Test to ensure that a ".info~" or ".info.bak" file doesn't end up
# in the distribution.  Bug report from Greg McGary.

. test-init.sh

cat >> configure.ac << 'END'
AC_OUTPUT
END

cat > Makefile.am << 'END'
info_TEXINFOS = textutils.texi subdir/main.texi
test: distdir
	@echo DISTFILES = $(DISTFILES)
	@case '$(DISTFILES)' in *'~'*|*'.bak'*) exit 1;; *) exit 0;; esac
	st=0; \
	 find $(distdir) | grep '~' && st=1; \
	 find $(distdir) | grep '\.bak' && st=1; \
	 exit $$st
PHONY: test
END

: > texinfo.tex
mkdir subdir
echo '@setfilename textutils.info' > textutils.texi
echo '@setfilename main.info' > subdir/main.texi

$ACLOCAL
$AUTOCONF
$AUTOMAKE

./configure
: > textutils.info
: > subdir/main.info
: > textutils.info~
: > textutils.info.bak
: > subdir/main.info~
: > subdir/main.info.bak
$MAKE test
$MAKE maintainer-clean
test -f subdir/main.info~
test -f subdir/main.info.bak

:
