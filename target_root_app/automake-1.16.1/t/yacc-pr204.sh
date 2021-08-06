#! /bin/sh
# Copyright (C) 2002-2018 Free Software Foundation, Inc.
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

# For PR 204.
# C sources derived from nodist_ yacc sources should not be distributed.
# See also related test 'yacc-nodist.sh'.
# The tests 'lex-nodist.sh' and 'lex-pr204.sh' does similar checks
# for lex-generated C files.

required='cc yacc'
. test-init.sh

cat >> configure.ac <<'EOF'
AM_MAINTAINER_MODE
AC_PROG_CC
AC_PROG_YACC
AC_OUTPUT
EOF

# The PARSE2 intermediate variable is there to make
# sure Automake match 'nodist_' against the right
# variable name...
cat > Makefile.am << 'EOF'
AM_YFLAGS = -d
EXTRA_PROGRAMS = foo
PARSE2 = parse2.y
nodist_foo_SOURCES = parse.y $(PARSE2)

distdirtest: distdir
	test ! -f $(distdir)/parse.c
	test ! -f $(distdir)/parse.y
	test ! -f $(distdir)/parse.h
	test ! -f $(distdir)/parse2.c
	test ! -f $(distdir)/parse2.y
	test ! -f $(distdir)/parse2.h
EOF

cat > parse.y << 'END'
%{
int yylex () {return 0;}
void yyerror (char *s) {}
%}
%%
maude : 'm' 'a' 'u' 'd' 'e' {};
END

cp parse.y parse2.y

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure
$MAKE distdirtest

# Make sure parse.c and parse2.c are still targets.
$MAKE parse.c parse2.c
test -f parse.c
test -f parse2.c

# Ensure the rebuild rule works despite AM_MAINTAINER_MODE, because
# it's a nodist_ parser.
$sleep
touch parse.y parse2.y
$sleep
$MAKE parse.c parse2.c
is_newest parse.c parse.y
is_newest parse2.c parse2.y

:
