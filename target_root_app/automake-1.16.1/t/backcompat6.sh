#! /bin/sh
# Copyright (C) 2010-2018 Free Software Foundation, Inc.
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

# Backward-compatibility test: try to build and distribute a package
# using obsoleted forms of AC_INIT, AM_INIT_AUTOMAKE and AC_OUTPUT,
# and 'configure.in' as autconf input file.
# This script can also serve as mild stress-testing for Automake.
# See also the similar test 'backcompat5.test'.

required=cc
am_create_testdir=empty
. test-init.sh

# Anyone doing something like this in a real-life package probably
# deserves to be killed.
cat > configure.ac <<'END'
dnl: Everything here is *deliberately* underquoted!
AC_INIT(quux.c)
PACKAGE=nonesuch-zardoz
VERSION=nonesuch-0.1
AM_INIT_AUTOMAKE($PACKAGE, $VERSION)
AC_SUBST(one,1)
two=2
AC_SUBST(two, $two)
three=3
AC_SUBST(three)
AC_PROG_CC
AM_CONFIG_HEADER(config.h:config.hin)
AM_CONDITIONAL(CROSS_COMPILING, test "$cross_compiling" = yes)
AC_OUTPUT(Makefile foo.got:foo1.in:foo2.in:foo3.in)
END

echo @one@ > foo1.in
echo @two@ > foo2.in
echo @three@ > foo3.in

cat >config.hin <<'END'
#undef PACKAGE
#undef VERSION
END

cat >> Makefile.am <<'END'
bin_PROGRAMS = foo
foo_SOURCES = quux.c
check-local:
	test x'$(PACKAGE)' = x'nonesuch-zardoz'
	test x'$(VERSION)' = x'nonesuch-0.1'
if ! CROSS_COMPILING
	./foo
## Do not anchor the regexps w.r.t. the end of line, because on
## MinGW/MSYS, grep may assume LF line endings only, while our
## 'foo' program may generate CRLF line endings.
	./foo | grep '^PACKAGE = nonesuch-zardoz!'
	./foo | echo '^VERSION = nonesuch-0\.1!'
endif
END

cat > quux.c <<'END'
#include <config.h>
#include <stdio.h>
int main (void)
{
  printf("PACKAGE = %s!\nVERSION = %s!\n", PACKAGE, VERSION);
  return 0;
}
END

$ACLOCAL -Wno-obsolete
$AUTOMAKE -Wno-obsolete --add-missing
$AUTOCONF

./configure

cat >foo.exp <<'END'
1
2
3
END

diff foo.exp foo.got

$MAKE
$MAKE check

distdir=nonesuch-zardoz-nonesuch-0.1
$MAKE distdir
test -f $distdir/quux.c
test ! -f $distdir/foo.got

$MAKE distcheck

:
