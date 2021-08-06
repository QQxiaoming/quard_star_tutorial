#! /bin/sh
# Copyright (C) 2003-2018 Free Software Foundation, Inc.
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

# Interaction of BUILT_SOURCES with conditionals.

. test-init.sh

cat >> configure.ac <<'END'
AM_CONDITIONAL([COND1], [test $cond1 = yes])
AM_CONDITIONAL([COND2], [test $cond2 = yes])
AC_OUTPUT
END

cat > Makefile.am << 'END'
if COND1
BUILT_SOURCES = a
else
BUILT_SOURCES = b
endif
if COND2
BUILT_SOURCES += c
endif

a b c:
	echo who cares > $@
END

$ACLOCAL
$AUTOMAKE
$AUTOCONF

cleanup ()
{
  # Files in $(BUILT_SOURCES) should be automatically removed
  # upon maintainer-clean.
  $MAKE maintainer-clean
  test ! -f a
  test ! -f b
  test ! -f c
}

./configure cond1=yes cond2=yes

$MAKE
test -f a
test ! -f b
test -f c

cleanup

./configure cond1=no cond2=yes

$MAKE
test ! -f a
test -f b
test -f c

cleanup

./configure cond1=yes cond2=no

$MAKE
test -f a
test ! -f b
test ! -f c

cleanup

./configure cond1=no cond2=no

$MAKE
test ! -f a
test -f b
test ! -f c

cleanup

:
