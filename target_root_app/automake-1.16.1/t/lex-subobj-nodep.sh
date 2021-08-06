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

# Ensure subdirs for subdir scanners are generated when subdir-objects
# are used, even when dependency tracking is disabled.

required='cc lex'
. test-init.sh

cat >>configure.ac <<\END
AC_PROG_CC
AC_PROG_LEX
AC_OUTPUT
END

cat >Makefile.am <<\END
AUTOMAKE_OPTIONS = subdir-objects
bin_PROGRAMS = p1 p2
p1_SOURCES = sub1/s1.l
p2_SOURCES = sub2/s2.l
p2_CPPFLAGS = -DWHATEVER
END

mkdir sub1 sub2

cat >sub1/s1.l <<\END
%{
#define YY_NO_UNISTD_H 1
%}
%%
"END"   return EOF;
.
%%
int main (void)
{
  while (yylex () != EOF)
    ;
  return 0;
}

int yywrap(void)
{
  return 1;
}
END

cp sub1/s1.l sub2/s2.l

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a
mkdir build
cd build
../configure --disable-dependency-tracking
$MAKE sub1/s1.c
$MAKE sub2/s2.c
rm -rf sub1 sub2
$MAKE

:
