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

# Test Objective C compilation flags.

. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_OBJC
AC_OUTPUT
END

cat > Makefile.am << 'END'
AM_DEFAULT_SOURCE_EXT = .m
bin_PROGRAMS = foo2 foo4
AM_OBJCFLAGS = -DVALUE=2 -DERROR=1
foo4_OBJCFLAGS = -DVALUE=4 -DERROR=1
END

for i in 2 4; do
  unindent > foo$i.m << END
    /* Let's make this file valid Objective C but invalid C. */
    #import <stdlib.h>
    @interface Who_Cares { } @end
    #ifdef ERROR
    #  error "macro ERROR is defined for foo.m"
    #else
    #  if VALUE == $i
         int main (void) { exit (0); }
    #  else
    #    error "VALUE is != $i in foo.m"
    #  endif
    #endif
END
done

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

$FGREP OBJCFLAGS Makefile.in # For debugging.
grep '\$(OBJCFLAGS).*\$(AM_OBJCFLAGS)'       Makefile.in && exit 1
grep '\$(OBJCFLAGS).*\$(foo.*_OBJCFLAGS)'    Makefile.in && exit 1
grep '\$(foo.*_OBJCFLAGS).*\$(AM_OBJCFLAGS)' Makefile.in && exit 1

./configure OBJCFLAGS=-UERROR
$MAKE

:
