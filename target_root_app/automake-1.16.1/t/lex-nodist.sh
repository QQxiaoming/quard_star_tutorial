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

# Checks for .c files derived from non-distributed .l sources.
# The test 'lex-pr204.sh' does similar check with AM_MAINTAINER_MODE
# enabled.
# The tests 'yacc-nodist.sh' and 'yacc-pr204.sh' does similar checks
# for yacc-generated .c and .h files.

required='cc lex'
. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
dnl Sister test 'lex-pr204.sh' should use 'AC_PROG_LEX' instead.
AM_PROG_LEX
AC_OUTPUT
END

cat > Makefile.am << 'END'
.PHONY: test-build test-dist
test-build: all
	ls -l
	test -f lexer.l
	test -f lexer.c
test-dist: distdir
	ls -l $(distdir)
	test ! -r $(distdir)/lexer.l
	test ! -r $(distdir)/lexer.c
check-local: test-build test-dist

lexer.l:
	rm -f $@ $@-t
	:; { : \
	  && echo '%{' \
	  && echo '#define YY_NO_UNISTD_H 1' \
	  && echo '%}' \
	  && echo '%%' \
	  && echo '"GOOD" return EOF;' \
	  && echo '.'; \
	} > $@-t
	chmod a-w $@-t && mv -f $@-t $@

bin_PROGRAMS = prog
prog_SOURCES = main.c
nodist_prog_SOURCES = lexer.l
prog_LDADD = $(LEXLIB)
CLEANFILES = $(nodist_prog_SOURCES)
END

cat > main.c << 'END'
int main ()
{
  return yylex ();
}

/* Avoid possible link errors. */
int yywrap (void)
{
  return 1;
}
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure
$MAKE
$MAKE test-build
$MAKE test-dist

# But the distribution must work correctly, assuming the user has
# the proper tools to process yacc files.
$MAKE distcheck

:
