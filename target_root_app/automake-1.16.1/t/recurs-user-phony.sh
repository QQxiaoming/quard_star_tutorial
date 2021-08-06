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

# Check that user-defined recursive targets and their associate
# '-local', '-am' and '-recursive' targets are declared as phony.

# Require GNU make, because some vendo makes (e.g., Solaris) doesn't
# truly respect .PHONY.
required=GNUmake
. test-init.sh

cat >> configure.ac <<'END'
AC_CONFIG_FILES([sub/Makefile])
AM_EXTRA_RECURSIVE_TARGETS([foo])
AC_OUTPUT
END

cat > Makefile.am <<'END'
SUBDIRS = sub
foo-local:
	echo 'GOOD!' > foo
END

mkdir sub
cat > sub/Makefile.am <<'END'
foo-local:
	echo 'GOOD!' > foo
END

dummy_files='
  foo
  foo-local
  foo-am
  foo-recursive
  sub/foo
  sub/foo-local
  sub/foo-am
'

$ACLOCAL
$AUTOCONF
$AUTOMAKE
./configure

touch $dummy_files

$MAKE foo
grep 'GOOD!' foo
grep 'GOOD!' sub/foo

:
