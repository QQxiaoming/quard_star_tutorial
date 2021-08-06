#! /bin/sh
# Copyright (C) 2012-2018 Free Software Foundation, Inc.
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

# Test that automatic determination of the linker works well with
# conditional use of languages in a single program.
# This currently doesn't truly work, but we have an easy workaround
# at least, that is tested here.
# See automake bug#11089.

required='cc c++'
. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AC_PROG_CXX
AM_CONDITIONAL([HAVE_CXX], [test $have_cxx = yes])
AC_OUTPUT
END

cat > Makefile.am << 'END'
bin_PROGRAMS = foo
if HAVE_CXX
foo_SOURCES = more.c++
else
foo_SOURCES = less.c
endif
## FIXME: ideally, this workaround shouldn't be needed.
if HAVE_CXX
foo_LINK = $(CXXLINK)
else
foo_LINK = $(LINK)
endif
END

$ACLOCAL
$AUTOMAKE
$AUTOCONF

rm -f *.c++
cat > less.c <<'END'
/* Valid C but deliberately invalid C++ */
main ()
{
  int new = 0;
  return new;
}
END

./configure have_cxx=no
run_make CXX=false

# Sanity check.
rm -f foo foo.exe
run_make CC=false && fatal_ '"make CC=false" succeeded unexpectedly'

$MAKE distclean

rm -f *.c
cat > more.c++ <<'END'
/* Valid C++ but deliberately invalid C */
using namespace std;
int main (void)
{
  return 0;
}
END

./configure have_cxx=yes
run_make CC=false

# Sanity check.
rm -f foo foo.exe
run_make CXX=false && fatal_ '"make CXX=false" succeeded unexpectedly'

:
