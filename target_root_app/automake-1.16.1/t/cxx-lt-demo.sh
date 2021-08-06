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

# Demo on Libtool/C++ support.

required='libtoolize c++'
am_create_testdir=empty
. test-init.sh

cat > configure.ac << 'END'
AC_INIT([GNU C++/Libtool Demo], [0.73], [bug-automake@gnu.org])
AC_CONFIG_SRCDIR([lib/libfoo.c++])
AC_CONFIG_AUX_DIR([ax])
AM_INIT_AUTOMAKE
AC_CANONICAL_HOST
AC_CANONICAL_BUILD
AC_PROG_CXX
AM_PROG_AR
LT_INIT
AC_CONFIG_FILES([
  Makefile
  src/Makefile
  lib/Makefile
  try.sh:try.in
])
AC_OUTPUT
END

mkdir ax lib src

cat > Makefile.am <<'END'
SUBDIRS = lib src

AUTOMAKE_OPTIONS = parallel-tests
TEST_EXTENSIONS = .sh
SH_LOG_COMPILER = $(SHELL) -ex
TESTS = try.sh

.PHONY: test-objs
check-local: test-objs
test-objs:
	test -f src/main.$(OBJEXT)
	test -f lib/libfoo.lo
END

cat > src/Makefile.am << 'END'
bin_PROGRAMS = zardoz
zardoz_SOURCES = main.cc
zardoz_LDADD = $(top_builddir)/lib/libfoo.la
AM_CPPFLAGS = -I$(top_builddir)/lib
END

cat > lib/Makefile.am << 'END'
lib_LTLIBRARIES = libfoo.la
nodist_libfoo_la_SOURCES = libfoo.h++
libfoo_la_SOURCES = libfoo.c++
libfoo.h++: $(srcdir)/libfoo.c++
	echo '#include <string>' >$@-t
	grep "target *(" "$(srcdir)/libfoo.c++" >>$@-t
	echo ';' >>$@-t
	chmod a-w $@-t && mv -f $@-t $@
BUILT_SOURCES = libfoo.h++
DISTCLEANFILES = $(BUILT_SOURCES)
END

cat > try.in << 'END'
#!/bin/sh
set -e
if test x"$host_alias" = x || test x"$build_alias" = x"$host_alias"; then
  ./src/zardoz
  test "`./src/zardoz`" = 'Howdy, Testsuite!'
else
  echo "Skip: cannot run a cross-compiled program"
  exit 77
fi
END

libtoolize --copy
$ACLOCAL
$AUTOCONF
$AUTOMAKE --add-missing --copy

ls -l . ax # For debugging.
# Ideally, the 'compile' script should not be required by C++ compilers.
# But alas, LT_INIT seems to invoke AC_PROG_CC anyway, and that brings in
# that script.
for f in ltmain.sh depcomp compile config.guess config.sub; do
  test -f ax/$f && test ! -h ax/$f || exit 1
done

cat > src/main.cc << 'END'
#include "libfoo.h++"
#include <iostream>
using namespace std;
int main (void)
{
  cout << "Howdy, " << target () << "!" << endl;
  return 0;
}
END

cat > lib/libfoo.c++ << 'END'
#include "libfoo.h++"
std::string target (void)
{
  std::string s1 = "Test";
  std::string s2 = "suite";
  return (s1 + s2);
}
END

./configure
run_make CC=false
ls -l . src lib # For debugging.
$MAKE test-objs
VERBOSE=yes $MAKE check-TESTS
grep 'Howdy.*Testsuite' try.log || grep 'Skip:.*cross-compiled' try.log

$MAKE distcheck

:
