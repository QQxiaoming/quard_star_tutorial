#! /bin/sh
# Copyright (C) 2011-2018 Free Software Foundation, Inc.
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

# Check that a user who wants to use an invalid prefix/primary
# combination can do so with a proper workaround.
# For example, this:
#   lib_PROGRAMS = foo
# is expected to cause an automake error, but this:
#   bardir = $(libdir)
#   bar_PROGRAMS = foo
# should work.

required=cc
. test-init.sh

cat >> configure.ac <<'END'
AC_PROG_CC
AM_PROG_AR
AC_PROG_RANLIB
AC_OUTPUT
END

cat > Makefile.am <<'END'
my_libdir = $(libdir)
my_lib_PROGRAMS = foo

foodir = $(bindir)
foo_LIBRARIES = libquux.a

xdir = $(libexecdir)
x_HEADERS = bar.h

installcheck-local: test
.PHONY: test
test:
	(cd '$(prefix)' && find .);: For debugging.
	ls -l '$(libdir)/foo$(EXEEXT)'
	test -f '$(libdir)/foo$(EXEEXT)'
	test -x '$(libdir)/foo$(EXEEXT)'
	ls -l '$(bindir)/libquux.a'
	test -f '$(bindir)/libquux.a'
	ls -l '$(libexecdir)/bar.h'
	test -f '$(libexecdir)/bar.h'
## If this test is run as root, "test -x" could succeed also for
## non-executable files, so we need to protect the next check.
## See automake bug#12041.
	if test -x Makefile; then echo SKIP THIS; else \
	  test ! -x '$(libexecdir)/bar.h'; \
	fi;
END

cat > foo.c <<'END'
int main (void)
{
  return 0;
}
END

cat > libquux.c <<'END'
int quux(void)
{
  return 1;
}
END

: > bar.h

$ACLOCAL
$AUTOMAKE -a
$AUTOCONF

cwd=$(pwd) || fatal_ "getting current working directory"
./configure --prefix="$cwd/_inst"

$MAKE install
$MAKE installcheck
$MAKE distcheck

:
