#! /bin/sh
# Copyright (C) 1996-2018 Free Software Foundation, Inc.
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

# Make sure that object names don't clash when using subdir-objects.
# The check is done for clashing programs, clashing libraries and
# a program that clashes with a library

. test-init.sh

mkdir src

cat >> configure.ac << 'END'
AC_PROG_CC
AC_PROG_RANLIB
AC_OUTPUT
END

cat > Makefile.am << 'END'
AUTOMAKE_OPTIONS = subdir-objects foreign
noinst_PROGRAMS =
noinst_LIBRARIES =

# CLASHING PROGRAMS
noinst_PROGRAMS += foo src/foo
foo_SOURCES = src/foo.c src/main.c
foo_CPPFLAGS = -DVAL=0
src_foo_CPPFLAGS = -DVAL=1
src_foo_SOURCES = src/foo.c src/main.c

# CLASHING LIBS
noinst_PROGRAMS += bar src/bar
noinst_LIBRARIES += libbar.a src/libbar.a
bar_SOURCES = src/main.c
bar_LDADD = libbar.a
src_bar_SOURCES = src/main.c
src_bar_LDADD = src/libbar.a
libbar_a_SOURCES = src/foo.c
libbar_a_CPPFLAGS = -DVAL=0
src_libbar_a_SOURCES = src/foo.c
src_libbar_a_CPPFLAGS = -DVAL=1

# CLASHING PROGRAM + LIB
noinst_PROGRAMS += libzap_a src/zap
noinst_LIBRARIES += src/libzap.a
libzap_a_SOURCES = src/main.c src/foo.c
libzap_a_CPPFLAGS = -DVAL=2
src_zap_SOURCES = src/main.c
src_zap_LDADD = src/libzap.a
src_libzap_a_SOURCES = src/foo.c
src_libzap_a_CPPFLAGS = -DVAL=3

# NON-CLASHING
noinst_PROGRAMS += src/foo-uniq
src_foo_uniq_SOURCES = src/main.c src/foo.c
src_foo_uniq_CPPFLAGS = -DVAL=4
END

cat > src/foo.c << 'END'
int
foo ()
{
  return VAL;
}
END

cat > src/main.c << 'END'
int foo (void);

int
main ()
{
  return foo ();
}
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE --add-missing

./configure
$MAKE
set +e
./foo || fail_ "./foo should return 0"
./src/foo; test $? = 1 || fail_ "./src/foo should return 1"
./bar || fail_ "./bar should return 0"
./src/bar; test $? = 1 || fail_ "./src/bar should return 1"
./libzap_a; test $? = 2  || fail_ "./libfoo_a should return 2"
./src/zap; test $? = 3 || fail_ "./src/prog_libfoo should return 3"
./src/foo-uniq; test $? = 4 || fail_ "./foo_uniq should return 4"
set -e
$MAKE clean
