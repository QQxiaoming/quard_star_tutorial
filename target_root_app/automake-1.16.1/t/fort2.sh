#! /bin/sh
# Copyright (C) 2006-2018 Free Software Foundation, Inc.
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

# Test that AC_FC_SRCEXT(f9x) works as intended:
# - $(FCFLAGS_f) will be used

# Cf. 'fort1.sh' and 'link_f90_only.sh'.

required=gfortran # Required only in order to run ./configure.
. test-init.sh

mkdir sub

cat >>configure.ac <<'END'
AC_PROG_FC
AC_FC_SRCEXT([f90])
AC_FC_SRCEXT([f95])
AC_FC_SRCEXT([f03])
AC_FC_SRCEXT([f08])
AC_FC_SRCEXT([blabla])
AC_OUTPUT
END

cat >Makefile.am <<'END'
AUTOMAKE_OPTIONS = subdir-objects
FC = fake-fc
bin_PROGRAMS = hello goodbye
hello_SOURCES = hello.f90 foo.f95 sub/bar.f95 hi.f03 sub/howdy.f03 \
                greets.f08 sub/bonjour.f08
goodbye_SOURCES = bye.f95 sub/baz.f90
goodbye_FCFLAGS = --gby
END

$ACLOCAL
$AUTOMAKE
grep '.\$(LINK)'       Makefile.in && exit 1
grep '.\$(FCLINK)'     Makefile.in
grep '.\$(FCCOMPILE)'  Makefile.in > stdout
cat stdout
grep -v '\$(FCFLAGS_f' stdout && exit 1
grep '.\$(FC.*\$(FCFLAGS_blabla' Makefile.in && exit 1

sed '/^AC_FC_SRCEXT.*blabla/d' configure.ac >t
mv -f t configure.ac

rm -rf autom4te*.cache
$ACLOCAL
$AUTOMAKE
$AUTOCONF

./configure

touch hello.f90 foo.f95 sub/bar.f95 hi.f03 sub/howdy.f03 greets.f08 \
      sub/bonjour.f08 bye.f95 sub/baz.f90

run_make -O -- -n \
  FCFLAGS_f90=--@90 FCFLAGS_f95=--@95 FCFLAGS_f03=--@03 FCFLAGS_f08=--@08
# To make it easier to have  stricter grepping below.
sed -e 's/[ 	][ 	]*/  /g' -e 's/^/ /' -e 's/$/ /' stdout > out
cat out

grep ' fake-fc .* --@90 .* hello\.f90 ' out
grep ' fake-fc .* --@95 .* foo\.f95 ' out
grep ' fake-fc .* --@95 .* sub/bar\.f95 ' out
grep ' fake-fc .* --@03 .* hi\.f03 ' out
grep ' fake-fc .* --@03 .* sub/howdy\.f03 ' out
grep ' fake-fc .* --@08 .* greets\.f08 ' out
grep ' fake-fc .* --@08 .* sub/bonjour\.f08 ' out
grep ' fake-fc .* --gby .* --@95 .*[` ]bye\.f95 ' out
grep ' fake-fc .* --gby .* --@90 .*[` ]sub/baz\.f90 ' out

test $(grep -c '.*--gby.*\.f' out) -eq 2

$EGREP 'fake-fc.*--@(95|03|08).*\.f90' out && exit 1
$EGREP 'fake-fc.*--@(90|03|08).*\.f95' out && exit 1
$EGREP 'fake-fc.*--@(90|95|08).*\.f03' out && exit 1
$EGREP 'fake-fc.*--@(95|95|03).*\.f08' out && exit 1

:
