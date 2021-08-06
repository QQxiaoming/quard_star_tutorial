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

# Test to ensure texinfo.tex is included in distribution.  Bug report by
# Jim Meyering.

required=makeinfo
. test-init.sh

cat >> configure.ac <<'END'
AC_OUTPUT
END

cat > Makefile.am << 'END'
info_TEXINFOS = textutils.texi
.PHONY: test1 test2
test1:
	@echo DISTFILES = $(DISTFILES)
	echo ' ' $(DISTFILES) ' ' | grep '[ /]texinfo\.tex '
test2: distdir
	ls -l $(distdir)
	test -f $(distdir)/texinfo.tex
END

echo '@setfilename textutils.info' > textutils.texi
: > texinfo.tex

$ACLOCAL
$AUTOCONF
$AUTOMAKE
./configure
$MAKE test1 test2

:
