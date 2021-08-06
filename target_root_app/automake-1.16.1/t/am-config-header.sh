#! /bin/sh
# Copyright (C) 2012-2018 Free Software Foundation, Inc.
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

# Check that the obsolete macro AM_CONFIG_HEADER still works.

. test-init.sh

cat > Makefile.am <<'END'
check-local:
	test -f oldconf.h
	test -f $(srcdir)/oldconf.in
END

cat >> configure.ac <<'END'
AM_CONFIG_HEADER([oldconf.h:oldconf.in])
AC_OUTPUT
END

$ACLOCAL -Wno-obsolete

$AUTOCONF -Werror -Wall 2>stderr && { cat stderr >&2; exit 1; }
cat stderr >&2
grep "^configure\.ac:4:.*'AM_CONFIG_HEADER'.*obsolete" stderr
grep "'AC_CONFIG_HEADERS'.* instead" stderr

$AUTOCONF -Werror -Wall -Wno-obsolete

$AUTOHEADER
test -f oldconf.in

$AUTOMAKE

./configure
$MAKE check-local
$MAKE distcheck

:
