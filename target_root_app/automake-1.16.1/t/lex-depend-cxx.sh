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

# Test to make sure dependencies work with Lex/C++.
# Test synthesized from PR automake/6.

required='c++ lex'
. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CXX
AM_PROG_LEX
AC_OUTPUT
END

cat > Makefile.am << 'END'
noinst_PROGRAMS = joe moe
joe_SOURCES = joe.ll
moe_SOURCES = moe.l++
LDADD = $(LEXLIB)

.PHONY: test-deps-exist
test-deps-exist:
	ls -l $(DEPDIR) ;: For debugging.
	test -f $(DEPDIR)/joe.Po
	test -f $(DEPDIR)/moe.Po

.PHONY: test-obj-updated
test-obj-updated: joe.$(OBJEXT) moe.$(OBJEXT)
	is_newest joe.$(OBJEXT) my-hdr.hxx
	is_newest moe.$(OBJEXT) my-hdr.hxx
END

cat > joe.ll << 'END'
%{
#define YY_DECL int yylex (void)
extern "C" YY_DECL;
#define YY_NO_UNISTD_H 1
int isatty (int fd) { return 0; }
%}
%%
"foo" return EOF;
.
%%
#include "my-hdr.hxx"
int yywrap (void)
{
  return 1;
}
int main (int argc, char **argv)
{
  return 0;
}
END

cp joe.ll moe.l++

cat > my-hdr.hxx <<'END'
// This header contains deliberately invalid C (but valid C++).
using namespace std;
END

$ACLOCAL
$AUTOMAKE -a

$FGREP joe.Po Makefile.in
$FGREP moe.Po Makefile.in

$AUTOCONF
# Try to enable dependency tracking if possible, even if that means
# using slow dependency extractors.
./configure --enable-dependency-tracking

# For debugging.
for f in $(find . -name '*.Po'); do
  cat $f
done

$MAKE test-deps-exist
$MAKE

$sleep
touch my-hdr.hxx
$MAKE test-obj-updated

:
