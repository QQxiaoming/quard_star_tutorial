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

# Test that automake works around a bug of Solaris Make. The bug is the
# following.  If we have a Makefile containg a file inclusion like this:
#   include .//foo.mk
# Solaris make fails with a message like:
#   make: ... can't find '/foo.mk': No such file or directory
#   make: fatal error ... read of include file '/foo.mk' failed
# (even if the file 'foo.mk' exists). The error disappear by collapsing
# the repeated slash '/' characters into a single one.
#
# See also sister "grepping" test 'subobj11b.sh', and related test
# 'subobj11c.sh'.

required=cc
. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AC_OUTPUT
END

cat > Makefile.am << 'END'
AUTOMAKE_OPTIONS = subdir-objects
bin_PROGRAMS = foo
## The './/' below is meant.
foo_SOURCES = .//src/foo.c
END

mkdir src

cat > src/foo.c << 'END'
int main(void)
{
  return 0;
}
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure --enable-dependency-tracking

depdir=$(sed -n 's/^ *DEPDIR *= *//p' Makefile)
if test x"$depdir" != x; then
  depdir=src/$depdir
else
  echo "$me: cannot extract value of DEPDIR from Makefile" >&2
  exit 1
fi

ls -l "$depdir"
test -f "$depdir"/foo.Po

echo 'quux:; echo "z@rd@z" >$@' >> "$depdir"/foo.Po

$MAKE quux
$FGREP "z@rd@z" quux

$MAKE

DISTCHECK_CONFIGURE_FLAGS='--enable-dependency-tracking' $MAKE distcheck
DISTCHECK_CONFIGURE_FLAGS='--disable-dependency-tracking' $MAKE distcheck

:
