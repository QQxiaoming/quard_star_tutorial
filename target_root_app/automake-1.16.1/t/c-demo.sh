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

# Demo on C support, also testing automatic dependency tracking,
# conditional SUBDIRS and convenience libraries.

required=cc
am_create_testdir=empty
. test-init.sh

cat > configure.ac << 'END'
AC_INIT([GNU C Demo], [22.3.2], [bug-automake@gnu.org])
AC_CONFIG_SRCDIR([tests/test.test])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE
AC_PROG_CC
AM_PROG_AR
AC_PROG_RANLIB
AM_CONDITIONAL([RUN_TESTS], [test x"$run_tests" != x"no"])
AC_CONFIG_FILES([Makefile src/Makefile lib/Makefile tests/Makefile])
AC_OUTPUT
END

if cross_compiling; then
  run_tests=no
else
  run_tests=yes
fi
export run_tests

mkdir build-aux lib src tests

cat > Makefile.am <<'END'
SUBDIRS = lib src

if RUN_TESTS
SUBDIRS += tests
endif

.PHONY: test-objs
check-local: test-objs
test-objs:
	test -f src/zardoz-main.$(OBJEXT)
	test -f lib/foo.$(OBJEXT)
	test -f lib/bar.$(OBJEXT)
END

cat > src/Makefile.am << 'END'
bin_PROGRAMS = zardoz
zardoz_SOURCES = main.c
zardoz_LDADD = $(top_builddir)/lib/lib-convenience.a
zardoz_CPPFLAGS = -I$(top_builddir)/lib -I$(top_srcdir)/lib
END

cat > lib/Makefile.am << 'END'
noinst_LIBRARIES = lib-convenience.a
lib_convenience_a_SOURCES = foo.c
lib_convenience_a_SOURCES += bar.c
dist_lib_convenience_a_SOURCES = bar.h
nodist_lib_convenience_a_SOURCES = foo.h

# We want this to be auto-generated an removed by "make clean", to
# ensure that cleaning rules work correctly; an older implementation
# of automatic dependency tracking support suffered of weaknesses in
# this situation, see the "historical comments" reported in:
# https://lists.gnu.org/archive/html/automake-patches/2012-06/msg00033.html
foo.h: $(srcdir)/foo.c
	sed -n 's/.*foo *(.*/&;/p' "$(srcdir)/foo.c" >$@-t
	test 1 -eq `wc -l <$@-t`
	chmod a-w $@-t && mv -f $@-t $@
BUILT_SOURCES = foo.h
CLEANFILES = $(BUILT_SOURCES)

check-local:
	test -f ${top_srcdir}/tests/test.test
END

cat > tests/Makefile.am << 'END'
AUTOMAKE_OPTIONS = parallel-tests
TEST_LOG_COMPILER = $(SHELL)
TESTS = test.test
EXTRA_DIST = $(TESTS)
END

cat > tests/test.test << 'END'
#!/bin/sh
set -x; set -e;
../src/zardoz
test "`../src/zardoz`" = 'Foo, Bar!'
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE --add-missing

test -f build-aux/depcomp
test -f build-aux/compile # We have per-target flags on C sources.

# Don't reject slow dependency extractors.
./configure --enable-dependency-tracking

cat > src/main.c << 'END'
#include "foo.h"
#include "bar.h"
int main (void)
{
  printf ("%s, %s!\n", foo (), bar ());
  return 0;
}
END

cat > lib/foo.c << 'END'
#include "foo.h"
static char s[4];
volatile char *foo (void)
{
  s[0] = 'F';
  s[1] = s[2] = 'o';
  s[3] = '\0';
  return s;
}
END

cat > lib/bar.c << 'END'
#include "bar.h"
const char *bar (void)
{
  return BARBAR;
}
END

cat > lib/bar.h << 'END'
#define BARBAR "Bar"
const char *bar (void);
END

$MAKE
ls -l . src lib # For debugging.
$MAKE test-objs

VERBOSE=x $MAKE check
if cross_compiling; then
  test ! -e tests/test-suite.log
  test ! -e tests/test.log
else
  test -f tests/test-suite.log
  grep 'Foo, Bar!' tests/test.log
fi

$MAKE distcheck

if ! cross_compiling && ! grep "[ $tab]depmode=none" Makefile; then
  # Let's check automatic dependency tracking.
  sed 's/^\(#define BARBAR \).*/\1 "Zap"/'  lib/bar.h > t
  mv -f t lib/bar.h
  $MAKE
  ./src/zardoz
  test "$(./src/zardoz)" = 'Foo, Zap!'
fi

$MAKE clean
test ! -e lib/foo.h
test -f lib/bar.h

:
