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

# Check that override of user-defined recursive targets work as
# expected.

. test-init.sh

cat >> configure.ac <<'END'
AC_CONFIG_FILES([sub/Makefile])
AM_EXTRA_RECURSIVE_TARGETS([foobar zardoz])
AC_OUTPUT
END

cat > Makefile.am <<'END'
SUBDIRS = sub
all-local: foobar zardoz
foobar:
	: > foobar.out
MOSTLYCLEANFILES = foobar.out
check-local:
	test -f foobar.out
	test ! -r sub/foobar.out
	test -f sub/zardoz.out
	test ! -r sub/baz.out
END

mkdir sub
cat > sub/Makefile.am <<'END'
foobar foobar-local:
	: > foobar.out
zardoz-local:
	: > baz.out
zardoz:
	: > zardoz.out
MOSTLYCLEANFILES = zardoz.out
END

$ACLOCAL
$AUTOCONF

AUTOMAKE_fails
grep '^Makefile\.am:3:.*user target.*foobar' stderr
grep '^Makefile\.am:.*foobar-local.*instead of foobar$' stderr
grep '^sub/Makefile\.am:5:.*user target.*zardoz' stderr
grep '^sub/Makefile\.am:.*zardoz-local.*instead of zardoz$' stderr

$AUTOMAKE -Wno-override

./configure

$MAKE check
$MAKE distcheck

:
