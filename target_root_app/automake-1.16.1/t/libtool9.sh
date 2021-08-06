#! /bin/sh
# Copyright (C) 2005-2018 Free Software Foundation, Inc.
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

# Make sure xxx_LINK is defined for each target that requires specific
# flags.
# Quite similar to 'libtool7.sh', using AM_LDFLAGS in addition to
# xxx_LDFLAGS.

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
AM_LDFLAGS = -module
lib_LTLIBRARIES = libmod1.la mod2.la
libmod1_la_SOURCES = mod1.c
libmod1_la_LDFLAGS =
libmod1_la_LIBADD = -dlopen mod2.la
mod2_la_SOURCES = mod2.c

bin_PROGRAMS = prg prg2
prg_SOURCES = prg.c
prg_LDADD = -dlopen libmod1.la -dlpreopen mod2.la
prg_CPPFLAGS = -DXYZ=1
prg2_SOURCES = prg.c
prg2_CFLAGS =

print:
	@echo 1BEG: $(prg_DEPENDENCIES) :END1
	@echo 2BEG: $(libmod1_la_DEPENDENCIES) :END2
	@echo 3BEG: $(libmod1_la_LINK) :END3
	@echo 4BEG: $(mod2_la_LINK) :END4
	@echo 5BEG: $(prg_LINK) :END5
	@echo 6BEG: $(prg2_LINK) :END6

END

mkdir liba

cat > mod1.c << 'END'
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

./configure
run_make -M -- print \
  LDFLAGS=ldflags \
  AM_LDFLAGS=am_ldflags \
  libmod1_la_LDFLAGS=lm1_la_ldflags \
  CFLAGS=cflags \
  AM_CFLAGS=am_cflags \
  prg2_CFLAGS=prg2_cflags

grep '1BEG: libmod1.la mod2.la :END1' output
grep '2BEG: mod2.la :END2' output
grep '3BEG:.* am_cflags cflags .*lm1_la_ldflags ldflags.* :END3' output
grep '3BEG: .*am_ldflags.* :END3' output && exit 1
grep '4BEG: :END4' output
grep '5BEG: :END5' output
grep '6BEG:.* prg2_cflags cflags .*am_ldflags ldflags.* :END6' output
grep '6BEG: .*am_cflags.* :END6' output && exit 1

$MAKE

:
