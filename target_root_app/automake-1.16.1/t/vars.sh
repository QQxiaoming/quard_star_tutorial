#! /bin/sh
# Copyright (C) 2001-2018 Free Software Foundation, Inc.
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

# Test various variable definitions that include an '=' sign.
# From Raja R Harinath.

. test-init.sh

cat > Makefile.am << 'END'
MY_FLAGS_1=-DABC=345
MY_FLAGS_2= -DABC=345
MY_FLAGS_3 =-DABC=345
MY_FLAGS_4 = -DABC=345
END

$ACLOCAL
$AUTOMAKE

for i in 1 2 3 4; do
  grep "^MY_FLAGS_$i *= *-DABC=345 *$" Makefile.in
done

:
