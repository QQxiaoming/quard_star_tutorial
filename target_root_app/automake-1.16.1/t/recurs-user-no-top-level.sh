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

# Check that user recursion works even for targets that don't exist
# in the top-level Makefile.

. test-init.sh

cat >> configure.ac <<'END'
AC_CONFIG_FILES([sub/Makefile])
AM_EXTRA_RECURSIVE_TARGETS([foo-bar])
AC_OUTPUT
END

mkdir sub

cat > Makefile.am <<'END'
SUBDIRS = sub
all-local: foo-bar
END

cat > sub/Makefile.am <<'END'
foo-bar-local:
	: > foo-bar
MOSTLYCLEANFILES = foo-bar
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE

./configure

$MAKE foo-bar
test -f sub/foo-bar

:
