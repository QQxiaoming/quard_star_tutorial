#! /bin/sh
# Copyright (C) 2013-2018 Free Software Foundation, Inc.
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

# Expose part of automake bug#13928: if the subdir-objects option is
# in use and a source file is listed in a _SOURCES variable with a
# leading $(srcdir) component, Automake will generate a Makefile that
# tries to create the corresponding object file in $(srcdir) as well.

required=cc
. test-init.sh

cat >> configure.ac <<'END'
AC_PROG_CC
AM_PROG_CC_C_O
AM_CONDITIONAL([OBVIOUS], [:])
AC_CONFIG_FILES([sub/Makefile])
AC_OUTPUT
END

cat > Makefile.am <<'END'
AUTOMAKE_OPTIONS = subdir-objects
SUBDIRS = sub

LESS = more

noinst_PROGRAMS = test test2
test_SOURCES = $(srcdir)/test.c

test2_SOURCES = $(indir)

indir =
if OBVIOUS
indir += ${srcdir}/$(LESS)/test.c
else
endif

test-objs:
	ls -la @srcdir@ .
        :
	test ! -f @srcdir@/test.$(OBJEXT)
	test -f test.$(OBJEXT)
	test ! -f @srcdir@/more/test.$(OBJEXT)
	test -f more/test.$(OBJEXT)
        :
	test ! -f @srcdir@/bar.$(OBJEXT)
	test -f bar.$(OBJEXT)
	test ! -f @srcdir@/baz.$(OBJEXT)
	test -f baz.$(OBJEXT)
        :
	test ! -d @srcdir@/$(DEPDIR)
	test ! -d @srcdir@/more/$(DEPDIR)
	test -d $(DEPDIR)
	test -d more/$(DEPDIR)

check-local: test-objs
END

mkdir sub
cat > sub/Makefile.am <<'END'
AUTOMAKE_OPTIONS = subdir-objects
bin_PROGRAMS = foo
foo_SOURCES = foo.h \
	      $(top_srcdir)/bar.c \
              ${top_srcdir}/baz.c
END


$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

mkfiles='Makefile.in sub/Makefile.in'
$EGREP '(test|ba[rz])\.|DEPDIR|dirstamp|srcdir' $mkfiles # For debugging.
$EGREP '\$.(top_)?srcdir./(test|ba[rz])\.[o$]' $mkfiles && exit 1
$FGREP '\$.(top_)?srcdir./.*$(am__dirstamp)' $mkfiles && exit 1
$FGREP '\$.(top_)?srcdir./.*$(DEPDIR)' $mkfiles && exit 1

cat > test.c <<'END'
int main (void)
{
  return 0;
}
END

mkdir more
cp test.c more/test.c

echo 'int foo (void);' > sub/foo.h

cat > bar.c <<'END'
#include "foo.h"
int main (void)
{
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
