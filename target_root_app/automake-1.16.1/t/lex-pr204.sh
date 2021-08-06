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

# Related to PR 204.
# C sources derived from nodist_ lex sources should not be distributed.
# See also related test 'lex-nodist.sh'.
# The tests 'yacc-nodist.sh' and 'yacc-pr204.sh' does similar checks
# for yacc-generated .c and .h files.

required='cc lex'
. test-init.sh

cat >> configure.ac <<'EOF'
AM_MAINTAINER_MODE
AC_PROG_CC
dnl We use AC_PROG_LEX deliberately.
dnl Sister 'lex-nodist.sh' should use 'AM_PROG_LEX' instead.
AC_PROG_LEX
AC_OUTPUT
EOF

# The LEXER2 intermediate variable is there to make sure Automake
# matches 'nodist_' against the right variable name...
cat > Makefile.am << 'EOF'
EXTRA_PROGRAMS = foo
LEXER2 = lexer2.l
nodist_foo_SOURCES = lexer.l $(LEXER2)

distdirtest: distdir
	test ! -f $(distdir)/lexer.c
	test ! -f $(distdir)/lexer.l
	test ! -f $(distdir)/lexer.h
	test ! -f $(distdir)/lexer2.c
	test ! -f $(distdir)/lexer2.l
	test ! -f $(distdir)/lexer2.h
EOF

cat > lexer.l << 'END'
%{
#define YY_NO_UNISTD_H 1
%}
%%
"GOOD"   return EOF;
.
%%
int main (void)
{
  return yylex ();
}
END

cp lexer.l lexer2.l

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure
$MAKE distdirtest

# Make sure lexer.c and lexer2.c are still targets.
$MAKE lexer.c lexer2.c
test -f lexer.c
test -f lexer2.c

# Ensure the rebuild rule works despite AM_MAINTAINER_MODE, because
# it's a nodist_ lexer.
$sleep
touch lexer.l lexer2.l
$sleep
$MAKE lexer.c lexer2.c
is_newest lexer.c lexer.l
is_newest lexer2.c lexer2.l

:
