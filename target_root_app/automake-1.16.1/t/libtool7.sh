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

# Make sure we allow Libtool's -dlopen/-dlpreopen
# Also check basic support for AM_LIBTOOLFLAGS/LIBTOOLFLAGS

required='cc libtoolize'
. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AM_PROG_AR
AC_LIBTOOL_DLOPEN
AM_PROG_LIBTOOL
AC_OUTPUT
END

cat > Makefile.am << 'END'
AUTOMAKE_OPTIONS = subdir-objects
AM_LIBTOOLFLAGS = --silent
lib_LTLIBRARIES = libmod1.la mod2.la
libmod1_la_SOURCES = sub/mod1.c
libmod1_la_LDFLAGS = -module
libmod1_la_LIBADD = -dlopen mod2.la
mod2_la_SOURCES = mod2.c
mod2_la_LDFLAGS = -module
mod2_la_LIBTOOLFLAGS =

bin_PROGRAMS = prg
prg_SOURCES = prg.c
prg_LDADD = -dlopen libmod1.la -dlpreopen mod2.la

.PHONY: print
print:
	@echo 1BEG: $(prg_DEPENDENCIES) :END1
	@echo 2BEG: $(libmod1_la_DEPENDENCIES) :END2
	@echo 3BEG: $(LTCOMPILE) :END3
END

mkdir sub liba

cat > sub/mod1.c << 'END'
int mod1 (void)
{
   return 1;
}
END

cat > mod2.c << 'END'
int mod2 (void)
{
   return 2;
}
END

cat > prg.c << 'END'
int main (void)
{
   return 0;
}
END

libtoolize --force --copy
$ACLOCAL
$AUTOCONF
$AUTOMAKE --add-missing --copy

./configure "--prefix=$(pwd)/_inst"

run_make -M print LIBTOOLFLAGS=--silent
grep '1BEG: libmod1.la mod2.la :END1' output
grep '2BEG: mod2.la :END2' output
grep '3BEG: .*silent.*silent.* :END3' output
test 2 -le $(grep mod2_la_LIBTOOLFLAGS Makefile | wc -l)

$MAKE

run_make -M install LIBTOOLFLAGS=--silent
grep 'silent.*silent.*prg' output
grep 'silent.*silent.*libmod1' output

run_make -M uninstall LIBTOOLFLAGS=--silent
grep 'silent.*silent.*libmod1' output

:
