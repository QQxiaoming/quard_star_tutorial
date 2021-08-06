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

# Automatic dependency tracking for Objective C++.
# See also sister test 'objc-deps.sh'.

. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_OBJCXX
AC_OUTPUT
END

cat > Makefile.am << 'END'
bin_PROGRAMS = foo
foo_SOURCES = bar.mm baz.h++ baz2.hh
END

cat > baz.h++ << 'END'
#include <iostream>
#include "baz2.hh"
END

cat > baz2.hh << 'END'
#define MSG "Hello, World"
END

cat > bar.mm << 'END'
/* The use of #import makes this valid Object C++ but invalid C++. */
#import "baz.h++"
int main (void)
{
    std::cout << MSG << "\n";
    return 0;
}
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE --add-missing

./configure --enable-dependency-tracking
$MAKE
cross_compiling || (./foo | $FGREP 'Hello, World') || exit 1

$sleep
: > old
echo '#define MSG "Howdy, Earth"' > baz2.hh
$MAKE
if test -f foo; then
  is_newest foo old
else
  is_newest foo.exe old
fi
cross_compiling || (./foo | $FGREP 'Howdy, Earth') || exit 1

$MAKE distcheck

:
