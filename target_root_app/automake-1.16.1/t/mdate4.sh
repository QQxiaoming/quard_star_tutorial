#! /bin/sh
# Copyright (C) 2001-2018 Free Software Foundation, Inc.
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

# Test to make sure that mdate-sh is added to the right directory.
# Report from Kevin Dalley.

. test-init.sh

cat >> configure.ac << 'END'
AC_CONFIG_FILES([sub/Makefile])
END

cat > Makefile.am << 'END'
SUBDIRS = sub
END

mkdir sub

cat > sub/Makefile.am << 'END'
info_TEXINFOS = textutils.texi
END

cat > sub/textutils.texi << 'END'
@include version.texi
@setfilename textutils.info
END

$ACLOCAL
$AUTOMAKE -a
ls -l sub
test -f sub/mdate-sh

:
