#! /bin/sh
# Copyright (C) 2013-2018 Free Software Foundation, Inc.
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

# Expose part of Automake bug#13928: config.status generates broken
# makefiles if the subdir-objects option is used and a _SOURCES
# variable lists sources whose directory part is specified as a
# make variable ("foo_SOURCES = $(src)/foo.c").

required=cc
. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AM_PROG_CC_C_O
AC_OUTPUT
END

cat > Makefile.am <<'END'
AUTOMAKE_OPTIONS = subdir-objects
src = s
bin_PROGRAMS = foo
foo_SOURCES = $(src)/foo.c
END

mkdir s
echo 'int main(void) { return 0; }' > s/foo.c

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure
$MAKE
test -f s/.deps/foo.Po
find . | $FGREP '$(src)' && exit 1
$MAKE distcheck

:
