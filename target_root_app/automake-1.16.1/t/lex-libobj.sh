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

# Check that we can provide a personal 'yywrap' function through the
# LIBOBJ machinery.

required='cc lex'
. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AC_PROG_RANLIB
AC_PROG_LEX
save_LIBS=$LIBS
LIBS="$LEXLIB $LIBS"
AC_REPLACE_FUNCS([yywrap])
LIBS=$save_LIBS
AC_OUTPUT
END

cat > Makefile.am << 'END'
noinst_PROGRAMS = foo
foo_SOURCES = foo.l
foo_LDADD = $(LEXLIB) $(LIBOBJS)
END

cat > yywrap.c << 'END'
int yywrap (void)
{
  return 1;
}
END

cat > foo.l <<'END'
%{
#define YY_NO_UNISTD_H 1
%}
%%
"END" return EOF;
.
%%
int main (void)
{
  return 0;
}
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure
grep LIBOBJS Makefile # For debugging.
$MAKE
$MAKE distclean

# Force "no system lex library".  Setting LEXLIB to a non-empty value
# ensures that configure won't search for a "lex library", and simply
# rely on the LEXLIB to provide it, if needed.  So, by setting LEXLIB
# to a blank but non-empty value we can fool configure into thinking
# that no system-level library providing a 'yywrap' function is
# available.  See also discussion on automake bug#11306.
./configure LEXLIB=' '
grep LIBOBJS Makefile # For debugging.
grep '^LIBOBJS *=.*yywrap.*\.o' Makefile # Sanity check.
$MAKE

yl_distcheck

:
