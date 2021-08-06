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
# See also "semantic" sister test 'subobj11a.sh', and related test
# 'subobj11c.sh'.

. test-init.sh

echo AC_PROG_CC >> configure.ac

cat > Makefile.am << 'END'
AUTOMAKE_OPTIONS = subdir-objects
bin_PROGRAMS = foo
## The 'zardoz' sources should activate a code paths in Automake that
## cannot be sensibly tested by sister test 'subobj11a.test'.  The other
## sources provide some sort of stress testing.
foo_SOURCES = \
  //server/zardoz0.c \
  //server//zardoz1.c \
  //server/path/to/zardoz2.c \
  //server/another//path///to////zardoz3.c \
  /foobar0.c \
  ///foobar1.c \
  ////foobar2.c \
  /sub///foobar3.c \
  ///sub/foobar4.c \
  .//foobar5.c \
  .//sub/foobar6.c \
  ./sub//foobar7.c \
  .//sub//foobar8.c \
  sub/sub//sub///sub////foobar9.c
END

$ACLOCAL
$AUTOMAKE -a

grep '\.P' Makefile.in # For debugging.

grep '//.*foobar\.P' Makefile.in && exit 1

# These checks depend on automake internals, but presently this is
# the only way to test the code path we are interested in.
# Please update these checks when (and if) the relevant automake
# internals are changed.
for x in zardoz0 zardoz1 path/to/zardoz2 another/path/to/zardoz3; do
  case $x in
   */*) d=$(echo $x | sed 's,[^/]*$,,'); b=$(echo $x | sed 's,^.*/,,');;
     *) d=''; b=$x;;
  esac
  # Be a little lax in the regexp, to account for automake conditionals,
  # quoting, and similar stuff.
  grep "[ 	]//server/$d\\\$(DEPDIR)/$b\\.Po" Makefile.in
done

# Sanity checks.
for i in 0 1 2 3 4 5 6 7 8 9; do
  grep "\$(DEPDIR)/foobar$i\\.Po" Makefile.in
done

:
