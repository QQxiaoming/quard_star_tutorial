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

# Test that POSIX variable expansion '$(var:str=rpl)' works when used
# with the PROGRAMS primary.

required=cc
. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AC_OUTPUT
END

cat > Makefile.am << 'END'
t1 = foo1 foo2
t2 = barx bar2

# Also try an empty match suffix, to ensure that the ':=' in there is
# not confused by the parser with an unportable assignment operator.
bin_PROGRAMS = $(t1:=-p) $(t2:2=y)

installcheck-local:
	ls -l $(prefix)/bin
	test -f $(prefix)/bin/foo1-p$(EXEEXT)
	test -x $(prefix)/bin/foo1-p$(EXEEXT)
	test -f $(prefix)/bin/foo2-p$(EXEEXT)
	test -x $(prefix)/bin/foo2-p$(EXEEXT)
	test -f $(prefix)/bin/barx$(EXEEXT)
	test -x $(prefix)/bin/barx$(EXEEXT)
	test -f $(prefix)/bin/bary$(EXEEXT)
	test -x $(prefix)/bin/bary$(EXEEXT)
END

for f in foo1-p.c foo2-p.c barx.c bary.c; do
  echo 'int main(void) { return 0; }' > $f
done

$ACLOCAL
$AUTOCONF
$AUTOMAKE

cwd=$(pwd) || fatal_ "getting current working directory"
./configure --prefix="$cwd/_inst"
$MAKE
$MAKE install
$MAKE installcheck
$MAKE distcheck

:
