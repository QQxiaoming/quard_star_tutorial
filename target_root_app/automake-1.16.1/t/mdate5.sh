#! /bin/sh
# Copyright (C) 1999-2018 Free Software Foundation, Inc.
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

# Test to make sure mdate-sh works correctly.

am_create_testdir=empty
. test-init.sh

get_shell_script mdate-sh

set x $(./mdate-sh mdate-sh)
shift
echo "$*" # For debugging.

# Check that mdate output looks like a date:
test $# = 3
case $1$3 in *[!0-9]*) exit 1;; esac
test $1 -lt 32
# Hopefully automake will be obsolete in 80 years ;-)
case $3 in 20[0-9][0-9]) :;; *) exit 1;; esac
case $2 in
  January|February|March|April|May|June|July|August) ;;
  September|October|November|December) ;;
  *) exit 1
esac

# Stricter checks on the year required a POSIX date(1) command.
if year=$(date +%Y) && test $year -gt 2010; then
  test $year = $3 || exit 1
fi

:
