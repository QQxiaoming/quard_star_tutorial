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

# Test that POSIX variable expansion '$(var:str=rpl)' works
# when used in LDADD.

required=cc
. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AC_PROG_RANLIB
AM_PROG_AR
AC_OUTPUT
END

cat > Makefile.am << 'END'
u = libquux1.lib libquux2.lib
v = libquux1
w = none

zardozdir = $(prefix)/zardoz
zardoz_PROGRAMS = foo bar

noinst_LIBRARIES = libquux1.a libquux2.a libquux3.a

# Also try an empty match suffix, to ensure that the ':=' in there is
# not confused by the parser with an unportable assignment operator.
LDADD = $(u:.lib=.a)
bar_LDADD = $(v:=.a) libquux2.a $(w:none=libquux3.a)

libquux1.c:
	echo 'int quux1(void) { return 0; }' > $@
CLEANFILES = libquux1.c # For FreeBSD make.
END

cat > foo.c <<'END'
int main(void)
{
  int quux1(void), quux2(void);
  return quux1() + quux2();
}
END

cat > bar.c <<'END'
int main(void)
{
  int quux1(void), quux2(void), quux3(void);
  return quux1() + quux2() + quux3();
}
END

echo 'int quux2(void) { return 0; }' > libquux2.c
echo 'int quux3(void) { return 0; }' > libquux3.c

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure
$MAKE
test -f libquux1.c
$MAKE distcheck

:
