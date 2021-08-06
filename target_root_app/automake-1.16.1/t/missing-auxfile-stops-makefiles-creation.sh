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

# Automake should stop the creation of Makefile.in files at the first
# encountered error due to a required auxiliary file not found.

. test-init.sh

cat >> configure.ac <<END
AM_PATH_PYTHON
END

echo SUBDIRS = > Makefile.am

for i in 1 2 3 4 5; do
  echo "AC_CONFIG_FILES([sub$i/Makefile])" >> configure.ac
  echo "SUBDIRS += sub$i" >> Makefile.am
  mkdir sub$i
  echo python_PYTHON = foo.py > sub$i/Makefile.am
done

: > sub1/Makefile.am

$ACLOCAL

unset AUTOMAKE_JOBS

AUTOMAKE_fails
ls -l Makefile.in */Makefile.in || : # For debugging.
# Top-level Makefile.in might be created or not, we don't want to set
# too strict semantics here, so don't check for it.  But the later
# files shouldn't exist.
test -f sub1/Makefile.in
test ! -e sub2/Makefile.in
test ! -e sub3/Makefile.in
test ! -e sub4/Makefile.in
test ! -e sub5/Makefile.in

rm -f Makefile.in */Makefile.in

: > sub2/Makefile.am
: > sub3/Makefile.am

AUTOMAKE_fails
ls -l Makefile.in */Makefile.in || : # For debugging.
test -f sub1/Makefile.in
test -f sub2/Makefile.in
test -f sub3/Makefile.in
test ! -e sub4/Makefile.in
test ! -e sub5/Makefile.in

:
