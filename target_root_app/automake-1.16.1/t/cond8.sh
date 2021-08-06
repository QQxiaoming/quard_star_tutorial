#! /bin/sh
# Copyright (C) 1999-2018 Free Software Foundation, Inc.
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

# Test to make sure _PROGRAMS conditionals can be written in a useful
# way.

. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AM_CONDITIONAL([X], [test "$x" = yes])
AC_OUTPUT
END

cat > Makefile.am << 'END'
if X
bin_PROGRAMS = x y
else
noinst_PROGRAMS = x y
endif

.PHONY: get-built get-install not-install
get-built:
	test -f x.$(OBJEXT)
	test -f y.$(OBJEXT)
	test -f x$(EXEEXT)
	test -f y$(EXEEXT)
get-installed:
	test -f $(bindir)/x$(EXEEXT)
	test -f $(bindir)/y$(EXEEXT)
not-installed:
	if find $(prefix) -type f | grep .; then exit 1; else :; fi
END

$ACLOCAL
$AUTOMAKE
$AUTOCONF

cat > x.c <<'END'
int main (void)
{
  return 0;
}
END

cp x.c y.c

instdir=$(pwd)/_inst || fatal_ "cannot get current directory"

# Skip the rest of the test in case of e.g. missing C compiler.
./configure --prefix="$instdir" x=yes || exit $?
$MAKE install
$MAKE get-built
$MAKE get-installed

$MAKE distclean
rm -rf _inst

./configure --prefix="$instdir" x=no
$MAKE install
$MAKE get-built
$MAKE not-installed

:
