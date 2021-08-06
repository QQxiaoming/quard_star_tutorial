#! /bin/sh
# Copyright (C) 1998-2018 Free Software Foundation, Inc.
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

# Yet another sources-in-conditional test.  Report from Tim Goodwin.

. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AM_CONDITIONAL([ONE], [true])
AM_CONDITIONAL([TWO], [false])
AC_OUTPUT
END

cat > Makefile.am << 'END'
bin_PROGRAMS = targ

if ONE
OPT_SRC = one.c
endif

if TWO
OPT_SRC = $(OPT_SRC) two.c
endif

targ_SOURCES = main.c $(OPT_SRC)
END

# The bug is that automake hangs.  So we give it an appropriate grace
# time, then kill it if necessary.
$ACLOCAL
$AUTOMAKE 2>stderr &
pid=$!

# MSYS bash seems to have a bug in kill, so don't try to kill too soon.
# The extra quoting avoids a maintainer-check failure.
sleep '2'

# Make at most 30 tries, one every 10 seconds (= 300 seconds = 5 min).
try=1
while test $try -le 30; do
  if kill -0 $pid; then
    : process $pid is still alive, wait and retry
    sleep '10'
    try=$(($try + 1))
  else
    cat stderr >&2
    # Automake must fail with a proper error message.
    grep 'variable.*OPT_SRC.*recursively defined' stderr
    exit 0
  fi
done
# The automake process probably hung.  Kill it, and exit with failure.
echo "$me: Automake process $pid hung"
kill $pid
exit 1
