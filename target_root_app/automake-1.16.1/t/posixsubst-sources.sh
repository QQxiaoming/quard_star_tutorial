#! /bin/sh
# Copyright (C) 2010-2018 Free Software Foundation, Inc.
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

# Test that POSIX variable expansion '$(var:str=rpl)' works when used
# with the SOURCES primary.

required=cc
. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AC_OUTPUT
END

cat > Makefile.am << 'END'
bin_PROGRAMS = foo

FOO = foo.cxx
BAR = bar__
BAZ = baz.

# Also try an empty match suffix, to ensure that the ':=' in there is
# not confused by the parser with an unportable assignment operator.
foo_SOURCES = main.c $(FOO:.cxx=.c)
dist_foo_SOURCES = $(BAR:__=.c)
nodist_foo_SOURCES = $(BAZ:=c)

bar.c baz.c:
	echo 'int $@ (void) { return 0; }' | sed 's/\.c //' > $@
CLEANFILES = baz.c
CLEANFILES += bar.c # For FreeBSD make.

.PHONY: test test2
check-local: test1 test2
test1:
	ls -l . $(srcdir)
	test -f $(srcdir)/bar.c
	test -f baz.c
test2: distdir
	ls -l $(distdir)
## These sources should be distributed ...
	test -f $(distdir)/bar.c
	test -f $(distdir)/foo.c
	test -f $(distdir)/main.c
## ... and this shouldn't.
	test ! -r $(distdir)/baz.c
END

cat > main.c <<'END'
int main(void)
{
  int foo(void), bar(void), baz(void);
  return foo() + bar() + baz();
}
END

echo 'int foo(void) { return 0; }' > foo.c

$ACLOCAL
$AUTOCONF
$AUTOMAKE

./configure
$MAKE
$MAKE test1 test2
$MAKE distcheck

:
