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

# Remake rules and user-defined recursive rules.

. test-init.sh

cat >> configure.ac << 'END'
m4_include([extra-targets.m4])
AC_CONFIG_FILES([sub/Makefile])
AC_OUTPUT
END

echo 'AM_EXTRA_RECURSIVE_TARGETS([foo])' > extra-targets.m4

cat > Makefile.am << 'END'
SUBDIRS = sub
all-local: foo
foo-local:
	: > foo
bar-local:
	: > bar
MOSTLYCLEANFILES = foo
END

mkdir sub
cat > sub/Makefile.am << 'END'
foo-local:
	: > foo2
MOSTLYCLEANFILES = foo2
bar-local: later-target
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE

./configure

$MAKE
test -f foo
test -f sub/foo2
test ! -f bar
test ! -f sub/zardoz
$MAKE distcheck

$sleep

echo 'AM_EXTRA_RECURSIVE_TARGETS([bar])' >> extra-targets.m4

cat >> Makefile.am <<'END'
MOSTLYCLEANFILES += bar oof
all-local: bar
foo-local: oof
oof:
	: > $@
END

echo 'include $(srcdir)/more.am' >> sub/Makefile.am

cat > sub/more.am << 'END'
later-target:
	: > zardoz
DISTCLEANFILES = zardoz
END

using_gmake || $MAKE Makefile
$MAKE
test -f foo
test -f oof
test -f sub/foo2
test -f bar
test -f sub/zardoz
$MAKE distcheck

:
