#! /bin/sh
# Copyright (C) 1999-2018 Free Software Foundation, Inc.
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

# Test to make sure texinfo.tex is correctly installed and disted by
# -a when we're using AC_CONFIG_AUX_DIR.  Bug report by by Per
# Cederqvist.

. test-init.sh

cat > configure.ac << END
AC_INIT([$me], [1.0])
AC_CONFIG_AUX_DIR([auxdir])
AM_INIT_AUTOMAKE
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
END

cat > Makefile.am << 'END'
info_TEXINFOS = textutils.texi
.PHONY: test1 test2
test1:
	@echo DISTFILES = $(DISTFILES)
	echo ' ' $(DISTFILES) ' ' | grep '[ /]auxdir/texinfo\.tex '
test2: distdir
	ls -l $(distdir)/*
	test -f $(distdir)/auxdir/texinfo.tex
END

echo '@setfilename textutils.info' > textutils.texi

mkdir auxdir

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

test -f auxdir/texinfo.tex

./configure

# Create textutils.info by hand, so that we don't have to require
# makeinfo.  Also ensure it's really newer than textutils.texi, so
# that make won't try to re-create it.
$sleep
: > textutils.info

$MAKE test1 test2

:
