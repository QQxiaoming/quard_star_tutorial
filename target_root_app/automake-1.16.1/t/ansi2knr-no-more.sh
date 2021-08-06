#! /bin/sh
# Copyright (C) 2011-2018 Free Software Foundation, Inc.
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

# Check that any attempt to use the obsolete de-ANSI-fication support
# is diagnosed.

. test-init.sh

warn_rx='automatic de-ANSI-fication.*removed'

echo AC_PROG_CC >> configure.ac
$ACLOCAL
mv aclocal.m4 aclocal.sav
cp configure.ac configure.sav

echo AM_C_PROTOTYPES >> configure.ac

$ACLOCAL -Wnone 2>stderr && { cat stderr >&2; exit 1; }
cat stderr >&2
grep "^configure\\.ac:5:.*$warn_rx" stderr

cat aclocal.sav "$am_top_srcdir"/m4/obsolete.m4 > aclocal.m4
$AUTOCONF -Wnone 2>stderr && { cat stderr >&2; exit 1; }
cat stderr >&2
grep "^configure\\.ac:5:.*$warn_rx" stderr

for opt in ansi2knr lib/ansi2knr; do
  # ansi2knr option in Makefile.am
  cp configure.sav configure.ac
  echo "AUTOMAKE_OPTIONS = $opt" > Makefile.am
  rm -rf autom4te*.cache
  AUTOMAKE_fails -Wnone
  grep "^Makefile\.am:1:.*$warn_rx" stderr
  # ansi2knr option in configure.ac
  : > Makefile.am
  sed "s|^\\(AM_INIT_AUTOMAKE\\).*|\1([$opt])|" configure.sav >configure.ac
  cat configure.ac # For debugging.
  rm -rf autom4te*.cache
  AUTOMAKE_fails -Wnone
  grep "^configure\\.ac:2:.*$warn_rx" stderr
done

:
