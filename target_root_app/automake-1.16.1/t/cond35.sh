#!/bin/sh
# Copyright (C) 2004-2018 Free Software Foundation, Inc.
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

# Check rules output for parser defined conditionally.
# Report from Roman Fietze.

required='cc lex yacc'
. test-init.sh

cat >>configure.ac <<'EOF'
AM_CONDITIONAL([CASE_A], [test -z "$case_B"])
AC_PROG_CC
AM_PROG_LEX
AC_PROG_YACC
AC_OUTPUT
EOF

cat > Makefile.am <<'EOF'
AM_YFLAGS               =       -d

BUILT_SOURCES           =       tparse.h

if CASE_A
bin_PROGRAMS            =       ta
ta_SOURCES              =       ta.c tparse.h tscan.l tparse.y
ta_LDADD                =       $(LEXLIB)
else
bin_PROGRAMS            =       tb
tb_SOURCES              =       tb.c tparse.h tscan.l tparse.y
tb_LDADD                =       $(LEXLIB)
endif


test-ta:
	test -f ta$(EXEEXT)
test-tb:
	test -f tb$(EXEEXT)
EOF

$ACLOCAL
$AUTOCONF
$AUTOMAKE --add-missing

$FGREP 'tparse.h' Makefile.in # For debugging.
test $($FGREP -c 'tparse.h:' Makefile.in) -eq 1

cat > tscan.l << 'END'
%{
#define YY_NO_UNISTD_H 1
%}
%%
"END"   return EOF;
%%
/* Avoid possible link errors. */
int yywrap (void)
{
  return 1;
}
END

cat > tparse.y << 'END'
%{
void yyerror (char *s) {}
%}
%token EOF
%%
foobar : 'f' 'o' 'o' 'b' 'a' 'r' EOF {};
END

cat > ta.c << 'END'
int main (void)
{
  return 0;
}
END

cp ta.c tb.c

./configure
$MAKE
$MAKE test-ta

./configure case_B=yes
$MAKE
$MAKE test-tb

:
