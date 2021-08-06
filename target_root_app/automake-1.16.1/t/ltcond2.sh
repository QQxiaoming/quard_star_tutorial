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

# Test for bug in conditionals.

required='cc native libtoolize'
. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AM_PROG_AR
AC_PROG_LIBTOOL
AC_SUBST([HELLO_SYSTEM], [hello-generic.lo])
AM_CONDITIONAL([LINUX], [true])
AC_OUTPUT
END

cat > Makefile.am << 'END'
lib_LTLIBRARIES = libhello.la
libhello_la_SOURCES = hello-common.c
EXTRA_libhello_la_SOURCES = hello-linux.c hello-generic.c
libhello_la_LIBADD = $(HELLO_SYSTEM)
libhello_la_DEPENDENCIES = $(HELLO_SYSTEM)

lib_LTLIBRARIES += libhello2.la
libhello2_la_SOURCES = hello-common.c
if LINUX
libhello2_la_SOURCES += hello-linux.c
else
libhello2_la_SOURCES += hello-generic.c
endif

bin_PROGRAMS = hello hello2
hello_SOURCES = main.c
hello_LDADD = libhello.la
hello2_SOURCES = main.c
hello2_LDADD = libhello2.la

check-local:
	./hello$(EXEEXT) | grep hello-generic
	./hello2$(EXEEXT) | grep hello-linux
	: > check-ok
END

cat > hello-linux.c <<'END'
const char* str = "hello-linux";
END

cat > hello-generic.c <<'END'
const char* str = "hello-generic";
END

cat > hello-common.c <<'END'
#include <stdio.h>
extern const char* str;
void print (void)
{
  puts (str);
}
END

cat > main.c <<'END'
int main (void)
{
  print();
  return 0;
}
END

libtoolize
$ACLOCAL
$AUTOCONF
$AUTOMAKE --add-missing
./configure
$MAKE check
test -f check-ok

:
