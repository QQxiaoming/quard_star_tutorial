#! /bin/sh
# Copyright (C) 2003-2018 Free Software Foundation, Inc.
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

# Make sure aclocal does not overwrite aclocal.m4 needlessly.
# Also make sure automake --no-force does not overwrite Makefile.in needlessly.

. test-init.sh

cat >> configure.ac << 'END'
SOME_DEFS
AC_CONFIG_FILES([sub/Makefile])
END

mkdir sub
: > sub/Makefile.am

cat >> Makefile.am << 'END'
SUBDIRS = sub
include fragment.inc
END

: > fragment.inc

mkdir m4
echo 'AC_DEFUN([SOME_DEFS], [])' > m4/somedefs.m4

# Automake will take aclocal.m4 to be newer if it has the same timestamp
# as Makefile.in.  Avoid the confusing by sleeping.
AUTOMAKE_after_aclocal ()
{
  $sleep
  $AUTOMAKE --no-force
}

# aclocal will rewrite aclocal.m4 unless the input files are all older than the
# existing aclocal.m4 -- sleep to ensure somedefs.m4 has an older timestamp
# than the aclocal.m4 that the next aclocal call will generate.
$sleep

$ACLOCAL -I m4
AUTOMAKE_after_aclocal

touch foo
$sleep
$ACLOCAL -I m4
AUTOMAKE_after_aclocal
# aclocal.m4 and Makefile.in should not have been updated, so 'foo'
# should be younger
is_newest foo aclocal.m4 Makefile.in sub/Makefile.in

$sleep
$ACLOCAL -I m4 --force
is_newest aclocal.m4 foo
# We still use --no-force for automake, but since aclocal.m4 has
# changed all Makefile.ins should be updated.
AUTOMAKE_after_aclocal
is_newest Makefile.in aclocal.m4 foo
is_newest sub/Makefile.in aclocal.m4 foo

$sleep
touch m4/somedefs.m4
$sleep
$ACLOCAL -I m4
AUTOMAKE_after_aclocal
# aclocal.m4 should have been updated, although its contents haven't
# changed.
is_newest aclocal.m4 m4/somedefs.m4
is_newest Makefile.in m4/somedefs.m4
is_newest sub/Makefile.in m4/somedefs.m4

$sleep
touch fragment.inc
$sleep
$ACLOCAL -I m4
AUTOMAKE_after_aclocal
# Only ./Makefile.in should change.
is_newest Makefile.in fragment.inc
is_newest fragment.inc aclocal.m4
is_newest fragment.inc sub/Makefile.in

grep README Makefile.in && exit 1

$sleep
: > README
$sleep
$AUTOMAKE --no-force
# Even if no dependency change, the content changed.
is_newest Makefile.in README
is_newest README sub/Makefile.in

grep README Makefile.in

: > sub/Makefile.in
$sleep
touch foo
$sleep
$ACLOCAL -I m4
$AUTOMAKE --no-force
# Only sub/Makefile.in should change.
is_newest foo aclocal.m4
is_newest foo Makefile.in
is_newest sub/Makefile.in foo

:
