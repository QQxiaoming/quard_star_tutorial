#! /bin/sh
# Copyright (C) 2011-2018 Free Software Foundation, Inc.
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

# Check that 'stamp-vti' is automatically distributed when info_TEXINFOS
# and version.texi are involved.
# Related to automake bug#7819.

required=makeinfo
. test-init.sh

cat >> configure.ac << END
AC_OUTPUT
END

cat > Makefile.am << 'END'
info_TEXINFOS = foo.texi
.PHONY: test
test: all distdir
	ls -l $(distdir)
	echo ' ' $(DIST_COMMON) ' ' | grep '[ /]stamp-vti '
	test -f $(distdir)/stamp-vti
END

cat > foo.texi << 'END'
\input texinfo
@setfilename foo.info
@settitle Zardoz
@node Top
@include version.texi
bar baz quux
@bye
END

# Required when using Texinfo.
: > texinfo.tex
cp "$am_scriptdir"/mdate-sh .

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure
$MAKE test

:
