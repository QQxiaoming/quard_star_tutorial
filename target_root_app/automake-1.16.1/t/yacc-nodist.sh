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

# Checks for .c and .h files derived from non-distributed yacc sources.
# The test 'yacc-pr204.sh' does similar check with AM_MAINTAINER_MODE
# enabled.
# The tests 'lex-nodist.sh' and 'lex-pr204.sh' does similar checks
# for lex-generated .c files.

required='cc yacc'
. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AC_PROG_YACC
AC_CONFIG_FILES([sub1/Makefile sub2/Makefile])
AC_OUTPUT
END

cat > Makefile.am << 'END'
SUBDIRS = sub1 sub2
.PHONY: test-build test-dist
test-build: all
	ls -l . sub1 sub2
	test -f sub1/parse.y
	test -f sub1/parse.c
	test -f sub2/parse.y
	test -f sub2/parse.c
	test -f sub2/parse.h
test-dist: distdir
	ls -l $(distdir) $(distdir)/sub1 $(distdir)/sub2
	test ! -r $(distdir)/sub1/parse.y
	test ! -r $(distdir)/sub1/parse.c
	test ! -r $(distdir)/sub1/parse.h
	test ! -r $(distdir)/sub2/parse.y
	test ! -r $(distdir)/sub2/parse.c
	test ! -r $(distdir)/sub2/parse.h
check-local: test-build test-dist
END

mkdir sub1 sub2

cat > sub1/Makefile.am << 'END'
parse.y:
	rm -f $@ $@-t
	:; { : \
	  && echo "%{" \
	  && echo "int yylex () { return 0; }" \
	  && echo "void yyerror (char *s) {}" \
	  && echo "%}" \
	  && echo "%%" \
	  && echo "maude : 'm' 'a' 'u' 'd' 'e' {}"; \
	} > $@-t
	chmod a-w $@-t && mv -f $@-t $@

bin_PROGRAMS = prog
prog_SOURCES = main.c
nodist_prog_SOURCES = parse.y
CLEANFILES = $(nodist_prog_SOURCES)
END

cat sub1/Makefile.am - > sub2/Makefile.am << 'END'
AM_YFLAGS = -d
BUILT_SOURCES = parse.h
END

cat > sub1/main.c << 'END'
int main ()
{
  return yyparse ();
}
END
cat - sub1/main.c > sub2/main.c << 'END'
#include "parse.h"
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure
$MAKE
$MAKE test-build
$MAKE test-dist

yl_distcheck

:
