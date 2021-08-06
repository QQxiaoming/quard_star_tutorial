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

# Stress test on canonicalization.

required='cc libtool libtoolize'
. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AM_PROG_AR
AC_PROG_RANLIB  dnl: for static libraries
AC_PROG_LIBTOOL dnl: for libtool libraries
AM_CONDITIONAL([CROSS_COMPILING], [test "$cross_compiling" = yes])
AC_OUTPUT
END

if touch ,foo-bar libb.az+baz lib~zardoz,,; then
  rm -f ,foo-bar libb.az+baz lib~zardoz,,
else
  skip_ "cannot create regular files with \"tricky\" names"
fi

cat > Makefile.am << 'END'
noinst_PROGRAMS = dummy_static dummy_dynamic ,foo-bar
noinst_LIBRARIES = libb.az+baz.a
noinst_LTLIBRARIES = lib~zardoz,,.la

dummy_static_SOURCES = dummy.c lib.h
dummy_dynamic_SOURCES = $(dummy_static_SOURCES)

dummy_static_LDADD = $(noinst_LIBRARIES)
dummy_dynamic_LDADD = $(noinst_LTLIBRARIES)

_foo_bar_SOURCES = foobar.c
libb_az_baz_a_SOURCES = libs.c
lib_zardoz___la_SOURCES = libd.c

check-local:
	ls -l
if CROSS_COMPILING
	test -f ./,foo-bar$(EXEEXT)
	test -f ./dummy_static$(EXEEXT)
	test -f ./dummy_dynamic$(EXEEXT)
else !CROSS_COMPILING
	./,foo-bar
	./dummy_static
	./dummy_dynamic
	./,foo-bar | grep 'Hello, FooBar!'
	./dummy_static | grep 'Hello from Static!'
	./dummy_dynamic | grep 'Hello from Dynamic!'
endif !CROSS_COMPILING
END

cat > foobar.c << 'END'
#include <stdio.h>
int main(void)
{
  printf("Hello, FooBar!\n");
  return 0;
}
END

cat > dummy.c << 'END'
#include <stdio.h>
#include "lib.h"
int main(void)
{
  printf("Hello from %s!\n", dummy_func());
  return 0;
}
END

echo 'const char *dummy_func(void);' > lib.h
echo 'const char *dummy_func(void) { return "Dynamic"; }' > libd.c
echo 'const char *dummy_func(void) { return "Static"; }' > libs.c

libtoolize
$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure

$MAKE check
$MAKE distcheck

:
