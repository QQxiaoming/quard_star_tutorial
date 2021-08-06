#! /bin/sh
# Copyright (C) 2015-2018 Free Software Foundation, Inc.
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

# Expose part of automake bug#13928, also for non-C languages: if the
# subdir-objects option is in use and a source file is listed in a
# _SOURCES variable with a leading $(srcdir) component, Automake will
# generate a Makefile that tries to create the corresponding object
# file in $(srcdir) as well.

required='cc c++ fortran77 fortran'
. test-init.sh

cat >> configure.ac <<'END'
AC_PROG_CC
AC_PROG_CXX
AC_PROG_F77
AC_PROG_FC
AM_CONDITIONAL([OBVIOUS], [:])
AC_CONFIG_FILES([sub/Makefile])
AC_OUTPUT
END

cat > Makefile.am <<'END'
AUTOMAKE_OPTIONS = subdir-objects
SUBDIRS = sub

LESS = m/o/r/e

noinst_PROGRAMS = test test2
test_SOURCES = $(srcdir)/test.f90

test2_SOURCES = $(indir)

indir = ${indir2} $(empty)
indir2 =
if OBVIOUS
indir2 += ${srcdir}/$(LESS)///test.f
else
endif

test-objs:
	ls -la @srcdir@ .
        :
	test ! -f @srcdir@/test.$(OBJEXT)
	test -f test.$(OBJEXT)
	test ! -f @srcdir@/m/o/r/e/test.$(OBJEXT)
	test -f m/o/r/e/test.$(OBJEXT)
        :
	test ! -f @srcdir@/bar.$(OBJEXT)
	test -f bar.$(OBJEXT)
	test ! -f @srcdir@/baz.$(OBJEXT)
	test -f baz.$(OBJEXT)
        :
	test ! -d @srcdir@/$(DEPDIR)
	test ! -d @srcdir@/m/o/r/e/$(DEPDIR)
	test -d $(DEPDIR)
	test -d m/o/r/e/$(DEPDIR)

check-local: test-objs
END

mkdir sub
cat > sub/Makefile.am <<'END'
AUTOMAKE_OPTIONS = subdir-objects
bin_PROGRAMS = foo
foo = baz
foo_SOURCES = foo.h \
	      $(top_srcdir)/bar.cc \
              ${top_srcdir}/$(foo).c
END


$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

mkfiles='Makefile.in sub/Makefile.in'
$EGREP '(test|ba[rz])\.|DEPDIR|dirstamp|srcdir' $mkfiles # For debugging.
$EGREP '\$.(top_)?srcdir./(test|ba[rz]|\$.foo.)\.[o$]' $mkfiles && exit 1
$FGREP '\$.(top_)?srcdir./.*$(am__dirstamp)' $mkfiles && exit 1
$FGREP '\$.(top_)?srcdir./.*$(DEPDIR)' $mkfiles && exit 1

cat > test.f90 <<'EOF'
      program foo
      stop
      end
EOF

mkdir -p m/o/r/e
cp test.f90 m/o/r/e/test.f

cat > sub/foo.h <<'END'
#ifdef __cplusplus
extern "C"
#endif
int foo (void);
END

cat > bar.cc <<'END'
#include "foo.h"
#include <iostream>
int main (void)
{
  std::cout << "OK!" << "\n";
  return foo ();
}
END

cat > baz.c <<'END'
#include "foo.h"
int foo (void)
{
  return 0;
}
END

mkdir build
cd build
../configure

$MAKE
$MAKE test-objs
$MAKE distcheck

:
