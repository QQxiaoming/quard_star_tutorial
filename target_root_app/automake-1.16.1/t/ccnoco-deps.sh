#! /bin/sh
# Copyright (C) 2013-2018 Free Software Foundation, Inc.
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

# Check that dependency tracking can also work with compilers that
# doesn't understand '-c -o', even if the AM_PROG_CC_C_O macro is not
# explicitly called.

required=gcc # For 'cc-no-c-o'.
. test-init.sh

echo '#define myStr "Hello"' > foobar.h

cat > foo.c << 'END'
#include <stdio.h>
#include "foobar.h"
int main (void)
{
  printf ("%s\n", myStr);
  return 0;
}
END

cat > Makefile.am <<'END'
bin_PROGRAMS = foo
foo_SOURCES = foo.c foobar.h
check-deps: all
	test -n '$(DEPDIR)' && test -d '$(DEPDIR)'
	ls -l $(DEPDIR)
	grep 'stdio\.h' $(DEPDIR)/foo.Po
	grep 'foobar\.h' $(DEPDIR)/foo.Po
check-updated: all
	is_newest foo$(EXEEXT) foobar.h
END

# We deliberately don't invoke AM_PROG_CC_C_O here.
cat >> configure.ac << 'END'
AC_PROG_CC
AC_OUTPUT
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE --add-missing

# Make sure the compiler doesn't understand '-c -o'.
CC=$am_testaux_builddir/cc-no-c-o; export CC

./configure >stdout || { cat stdout; exit 1; }
cat stdout
$EGREP 'understands? -c and -o together.* no$' stdout
grep '^checking dependency style .*\.\.\. gcc' stdout

$MAKE check-deps

if ! cross_compiling; then
  ./foo
  test "$(./foo)" = Hello
fi

$sleep
echo '#define myStr "Howdy"' > foobar.h
$MAKE check-updated

if ! cross_compiling; then
  ./foo
  test "$(./foo)" = Howdy
fi

:
