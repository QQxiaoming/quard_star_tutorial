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

# Test to make sure automatic dependency tracking work with Lex/C.
# Test suggested by PR automake/6.

required='cc lex'
. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AM_PROG_LEX
AC_OUTPUT
END

cat > Makefile.am << 'END'
bin_PROGRAMS = zoo
zoo_SOURCES = joe.l
LDADD = $(LEXLIB)

.PHONY: test-deps-exist
test-deps-exist:
	ls -l $(DEPDIR) ;: For debugging.
	test -f $(DEPDIR)/joe.Po

.PHONY: test-obj-updated
test-obj-updated: joe.$(OBJEXT)
	is_newest joe.$(OBJEXT) my-hdr.h
END

cat > joe.l << 'END'
%{
#define YY_NO_UNISTD_H 1
%}
%%
"foo" return EOF;
.
%%
#include "my-hdr.h"
int main (void)
{
  printf("%s\n", MESSAGE);
  return 0;
}
/* Avoid possible link errors. */
int yywrap (void)
{
  return 1;
}
END

cat > my-hdr.h <<'END'
#include <stdio.h>
#define MESSAGE "Hello, World!"
END

$ACLOCAL
$AUTOMAKE -a

$FGREP joe.Po Makefile.in

$AUTOCONF
# Try to enable dependency tracking if possible, even if that means
# using slow dependency extractors.
./configure --enable-dependency-tracking

$MAKE test-deps-exist
$MAKE
cross_compiling || test "$(./zoo)" = 'Hello, World!' || exit 1

$sleep
cat >> my-hdr.h << 'END'
#undef MESSAGE
#define MESSAGE "Howdy, Earth!"
END
$MAKE test-obj-updated
$MAKE
cross_compiling || test "$(./zoo)" = 'Howdy, Earth!' || exit 1

:
