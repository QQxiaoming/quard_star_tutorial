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

# mdate-sh can work with weird file names, doesn't eval too much.

am_create_testdir=empty
. test-init.sh

a=
file='file  name $a'
( : > "$file" ) \
  || skip_ "file name with spaces and metacharacters not accepted"

get_shell_script mdate-sh

./mdate-sh "$file" >stdout 2>stderr ||
  { cat stdout; cat stderr >&2; exit 1; }
cat stdout
cat stderr >&2
grep '[12][0-9]\{3\}' stdout
test ! -s stderr

:
