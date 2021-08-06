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

# Verify our probe that checks that "rm -f" doesn't complain if called
# without file operands works as expected.  See automake bug#10828.

. test-init.sh

echo AC_OUTPUT >> configure.ac
: > Makefile.am

$ACLOCAL
$AUTOCONF
$AUTOMAKE

mkdir bin
cat > bin/rm <<'END'
#!/bin/sh
set -e; set -u;
PATH=$original_PATH; export PATH
rm_opts=
while test $# -gt 0; do
  case $1 in
    -*) rm_opts="$rm_opts $1";;
     *) break;;
  esac
  shift
done
if test $# -eq 0; then
  echo "Oops, fake rm called without arguments" >&2
  exit 1
else
  exec rm $rm_opts "$@"
fi
END
chmod a+x bin/rm

original_PATH=$PATH
PATH=$(pwd)/bin$PATH_SEPARATOR$PATH
export PATH original_PATH

rm -f && exit 99 # Sanity check.

./configure 2>stderr && { cat stderr >&2; exit 1; }
cat stderr >&2

grep "'rm' program.* unable to run without file operands" stderr
$FGREP "tell bug-automake@gnu.org about your system" stderr
$FGREP "install GNU coreutils" stderr
$EGREP "(^| |')ACCEPT_INFERIOR_RM_PROGRAM($| |')" stderr

ACCEPT_INFERIOR_RM_PROGRAM=yes; export ACCEPT_INFERIOR_RM_PROGRAM

./configure
$MAKE
$MAKE distcheck

# For the sake of our exit trap.
PATH=$original_PATH; export PATH

:
