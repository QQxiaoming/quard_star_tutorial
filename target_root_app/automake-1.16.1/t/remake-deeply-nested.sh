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

# Test remake rules in deeply nested subdirectories, and with a lot
# of files.  This is basically a mild stress test, ensuring that rebuild
# rules don't break in obvious ways in a slightly "heavier than usual"
# setup.

. test-init.sh

ocwd=$(pwd) || fatal_ "getting current working directory"

magic1=::MagicStringOne::
magic2=__MagicStringTwo__

echo "AC_SUBST([FOO], [$magic1])" >> configure.ac

echo "@FOO@" > bar.in
echo "AC_CONFIG_FILES([bar])" >> configure.ac
unset d # Avoid unduly interferences from the environment.
for i in 0 1 2 3 4 5 6 7 8 9; do
  d=${d+"$d/"}sub$i
  echo "SUBDIRS = sub$i" > Makefile.am
  echo "AC_CONFIG_FILES([$d/Makefile])" >> "$ocwd"/configure.ac
  echo "AC_CONFIG_FILES([$d/bar])" >> "$ocwd"/configure.ac
  mkdir sub$i
  cd sub$i
  echo "$d: @FOO@" > bar.in
done
echo AC_OUTPUT >> "$ocwd"/configure.ac
: > Makefile.am
unset d i

bottom=$(pwd) || fatal_ "getting current working directory"

cd "$ocwd"

makefiles_am_list=$(find . -name Makefile.am | LC_ALL=C sort)
makefiles_list=$(echo "$makefiles_am_list" | sed 's/\.am$//')
bar_in_list=$(find . -name bar.in | LC_ALL=C sort)
bar_list=$(echo "$bar_in_list" | sed 's/\.in$//')

cat configure.ac # For debugging.

$ACLOCAL
$AUTOMAKE
$AUTOCONF

./configure

for f in configure config.status $makefiles_list $bar_list; do
  $FGREP "$magic1" $f
done

$sleep
sed "s/$magic1/$magic2/" configure.ac >configure.tmp
mv -f configure.tmp configure.ac

cd "$bottom"
$MAKE
cd "$ocwd"

for f in configure config.status $makefiles_list $bar_list; do
  $FGREP "$magic2" $f
done
$FGREP "$magic1" configure config.status $makefiles_list $bar_list && exit 1

$MAKE distcheck

:
