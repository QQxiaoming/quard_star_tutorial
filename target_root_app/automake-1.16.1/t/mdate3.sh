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

# Test to make sure mdate-sh run correctly.

. test-init.sh

cat > configure.ac << END
AC_INIT([$me], [1.0])
AC_CONFIG_AUX_DIR([foo])
AM_INIT_AUTOMAKE
AC_CONFIG_FILES([Makefile])
END

cat > Makefile.am << 'END'
info_TEXINFOS = textutils.texi
END

cat > textutils.texi << 'END'
@include version.texi
@setfilename textutils.info
END

mkdir foo

# Required when using Texinfo.
: > foo/texinfo.tex
: > foo/mdate-sh
mv install-sh foo
mv missing foo

$ACLOCAL
$AUTOMAKE

grep '[^/]mdate-sh' Makefile.in && exit 1
exit 0
