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

# If the last line of a automake-rewritten definition is made only of
# @substitutions@, automake should take care of appending an empty
# variable to make sure that line cannot end up substituted as a blank
# line (that would confuse HP-UX Make).
# These checks have been introduced in commit 'Release-1-9-254-g9d0eaef'
# into the former test 'subst2.test'.

. test-init.sh

# These are deliberately quite long, so that the xxx_PROGRAMS definition
# in Makefile.am below will be split on multiple lines, with the last
# line containing only @substituted@ stuff that expands to empty (this is
# required to expose the bug we are testing).
v1=ABCDEFGHIJKLMNOPQRSTUVWX
v2=ABCDEFGHIJKLMNOPQRSTUVWXY
v3=ABCDEFGHIJKLMNOPQRSTUVWXYZ

# Literal backslash for use by grep.
bs='\\'

cat >> configure.ac <<END
AC_SUBST([A], [''])
AC_SUBST([$v1], [''])
AC_SUBST([$v2], [''])
AC_SUBST([$v3], [''])
AC_OUTPUT
END

cat >Makefile.am <<END
AUTOMAKE_OPTIONS = no-dependencies
CC = false
EXEEXT =

## The "x" and "zardoz" strings and the use of '+=' are there to ensure
## that these variables get rewritten by Automake.
noinst_PROGRAMS = x @$v1@ @$v2@ @$v3@
bin_PROGRAMS = @A@
bin_PROGRAMS += @$v1@ @$v2@ @$v3@
check_PROGRAMS = zardoz \$(noinst_PROGRAMS)

## Required whenever there are @substituted@ values in the
## PROGRAMS primary, otherwise automake will complain.
EXTRA_PROGRAMS =

test-real-empty:
	is \$(noinst_PROGRAMS) == x
	is \$(bin_PROGRAMS)    ==
	is \$(check_PROGRAMS)  == zardoz x

test-fake-empty:
	is \$(noinst_PROGRAMS) == x X
	is \$(bin_PROGRAMS)    == X
	is \$(check_PROGRAMS)  == zardoz x X
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE
# For debugging.
$EGREP -n 'ABCD|am__empty' Makefile.in
# Sanity check.
test $($EGREP -c "^[ $tab]*@$v2@ @$v3@[ $tab]*$bs?$" Makefile.in) -eq 3

./configure
{
  sed -n '/^noinst_PROGRAMS *=/,/[^\\]$/p' Makefile
  sed -n '/^bin_PROGRAMS *=/,/[^\\]$/p' Makefile
  sed -n '/^check_PROGRAMS *=/,/[^\\]$/p' Makefile
  sed -n '/^am__EXEEXT.*=/,/[^\\]$/p' Makefile
} >t-programs
cat t-programs
grep '^ *$' t-programs && exit 1

run_make test-real-empty
run_make test-fake-empty am__empty=X

:
