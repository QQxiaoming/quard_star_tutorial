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

# Check that many user-defined recursive targets can be supported
# at once, and that calls to 'AM_EXTRA_RECURSIVE_TARGETS' are
# cumulative.

. test-init.sh

cat >> configure.ac <<'END'
AC_CONFIG_FILES([sub/Makefile])
## NOTE: extra white spaces, tabs, newlines and backslashes in the
## lines below: on purpose.
AM_EXTRA_RECURSIVE_TARGETS([ foo  \
	bar  ])
AC_SUBST([CLEANFILES], ['foo bar baz'])
AC_OUTPUT
# Yes, this appears after AC_OUTPUT.  So what?
AM_EXTRA_RECURSIVE_TARGETS([baz])
END

mkdir sub

cat > Makefile.am <<'END'
SUBDIRS = sub
foo-local:
	: > foo
bar-local:
	echo x > bar
baz-local: ; touch baz
check-local: foo bar baz
	ls -l . sub ;: For debugging.
	test -f foo
	test -f bar
	test -f baz
	test -f sub/foo
	test -f sub/bar
	test -f sub/baz
END

cat > sub/Makefile.am <<'END'
foo-local bar-local baz-local:
	touch `echo $@ | sed 's/-local$$//'`
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE

for t in foo bar baz; do
  $FGREP "$t-am" Makefile.in
  $FGREP "$t-am" sub/Makefile.in
done

./configure

$MAKE check
$MAKE distcheck

:
