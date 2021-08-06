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

# Check that there are no duplicates in the list of files automatically
# distributed by automake.

. test-init.sh

re='Files .*automatically distributed.*if found'

# The automake manual states that the list of automatically-distributed
# files should be given by 'automake --help'.

list1=$($AUTOMAKE --help \
         | sed -n "/^$re.*always/,/^ *$/p" \
         | sed 1d)
list1=$(echo $list1)

list2=$($AUTOMAKE --help \
         | sed -n "/^$re.*under certain conditions/,/^ *$/p" \
         | sed 1d)
list2=$(echo $list2)

test -n "$list1"
test -n "$list2"

st=0
for i in 1 2; do
  eval list=\${list$i}
  for f in $list; do echo $f; done | sort > files$i.lst
  uniq files$i.lst > files$i.uniq
  cat files$i.lst
  cat files$i.uniq
  diff files$i.lst files$i.uniq || st=1
done

exit $st
