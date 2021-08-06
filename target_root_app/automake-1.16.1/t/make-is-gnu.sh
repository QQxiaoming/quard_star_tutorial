#! /bin/sh
# Copyright (C) 2013-2018 Free Software Foundation, Inc.
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

# Check that $(am__is_gnu_make) can be used to correctly determine if
# we are running under GNU make.

. test-init.sh

if using_gmake; then
 as_expected () { test $1 -eq 0 && test -f ok && test ! -e ko; }
else
 as_expected () { test $1 -gt 0 && test -f ko && test ! -e ok; }
fi

echo AC_OUTPUT >> configure.ac

cat > Makefile.am <<'END'
all: file
	$(am__is_gnu_make)
file:
	if $(am__is_gnu_make); then : > ok; else : > ko; fi
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE
./configure

st=0; $MAKE || st=$?
if using_gmake; then
 test $st -eq 0
 test -f ok
 test ! -e ko
else
 test $st -gt 0
 test -f ko
 test ! -e ok
fi

rm -f ok ko

run_make -M -- -s file
if using_gmake; then
 test -f ok
 test ! -e ko
else
 test -f ko
 test ! -e ok
fi
test ! -s output

:
