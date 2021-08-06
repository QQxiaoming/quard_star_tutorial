#! /bin/sh
# Copyright (C) 2010-2018 Free Software Foundation, Inc.
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

# Check that distributed broken symlinks cause 'make dist' to fail, and
# to do so with (mostly) meaningful diagnostic.

. test-init.sh

# We need, for our broken symlinks, names that make it hard to get false
# positives when grepping make output to look for them.
lnk_base=BrknSymlnk
lnk1=${lnk_base}__001
lnk2=${lnk_base}__002
lnka=${lnk_base}__aaa
lnkb=${lnk_base}__bbb

ln -s nonesuch $lnk1 || skip_ "cannot create broken symlinks"

ln -s "$(pwd)/nonesuch" $lnk2

ln -s $lnk1 $lnka
ln -s $lnka $lnkb

# Sanity checks.
test ! -e $lnk1
test ! -e $lnk2
test ! -e $lnka
test ! -e $lnkb
test -h $lnk1
test -h $lnk2
test -h $lnka
test -h $lnkb

cat >> configure.ac <<'END'
AC_OUTPUT
END

$ACLOCAL
$AUTOCONF

ls -l # For debugging.

# Don't try to use "make -k", because some botched make implementations
# (HP-UX, IRIX) might still exit on the first error in this situations.
for lnk in $lnk1 $lnk2 $lnka $lnkb; do
  echo "EXTRA_DIST = $lnk" > Makefile.am
  $AUTOMAKE
  ./configure
  # Distribution must fail, with a decent error message.
  run_make -M -e FAIL distdir
  $FGREP $lnk output
done

:
