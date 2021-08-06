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

# Check that user recursion works with various types of indirections
# *involved in the definition of the '*-local' targets*: make macros,
# AC_SUBST'd strings, automake-time file inclusions, automake
# conditionals...

. test-init.sh

cat >> configure.ac <<'END'
AC_CONFIG_FILES([
    sub1/Makefile
    sub1/subsub/Makefile
    sub2/Makefile
    sub2/subsub/Makefile
])
AM_EXTRA_RECURSIVE_TARGETS([foo])
AC_SUBST([FOO_LOCAL], [foo-local])
AM_CONDITIONAL([COND], [:])
AC_OUTPUT
END

mkdir sub1 sub1/subsub sub2 sub2/subsub

cat > Makefile.am <<'END'
SUBDIRS = sub1 sub2
AM_FOO_LOCAL = foo-local

$(AM_FOO_LOCAL):
	pwd && : > foo
CLEANFILES = foo

all-local: foo
check-local:
	test -f foo
	test -f sub1/foo
	test -f sub1/subsub/foo
	test -f sub2/foo
	test -f sub2/subsub/foo
	test ! -r sub2/subsub/bar
END

cat > sub1/Makefile.am <<'END'
SUBDIRS = subsub
@FOO_LOCAL@:
	pwd && : > foo
CLEANFILES = foo
END

cat > sub1/subsub/Makefile.am <<'END'
$(FOO_LOCAL):
	pwd && : > foo
CLEANFILES = foo
END

cat > sub2/Makefile.am <<'END'
include $(srcdir)/bar.am
include $(srcdir)/baz.am
CLEANFILES = foo
END

echo 'SUBDIRS = subsub' > sub2/bar.am
echo 'foo-local: ; pwd && : > foo' > sub2/baz.am

cat > sub2/subsub/Makefile.am <<'END'
if COND
foo-local:
	pwd && : > foo
CLEANFILES = foo
else !COND
foo-local:
	pwd && : > bar
endif !COND
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE

./configure

$MAKE check
$MAKE distcheck

:
