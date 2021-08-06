#! /bin/sh
# Copyright (C) 2001-2018 Free Software Foundation, Inc.
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

# Test to make sure we can compile libtool libraries when the compiler
# doesn't understand '-c -o'.

required='gcc libtoolize' # For cc-no-c-o.
. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AM_PROG_AR
LT_INIT
$CC --version
$CC -v
AC_OUTPUT
END

cat > Makefile.am << 'END'
lib_LTLIBRARIES = libwish.la
END

cat > libwish.c << 'END'
int wish_granted (void)
{
   return 0;
}
END

# Make sure the compiler doesn't understand '-c -o'.
CC=$am_testaux_builddir/cc-no-c-o; export CC

libtoolize --verbose --install
$ACLOCAL
$AUTOCONF
$AUTOMAKE --copy --add-missing

for vpath in : false; do
  if $vpath; then
    srcdir=..
    mkdir build
    cd build
  else
    srcdir=.
  fi
  $srcdir/configure >stdout || { cat stdout; exit 1; }
  cat stdout
  $EGREP 'understands? -c and -o together.* no$' stdout
  # No repeated checks please.
  test $(grep ".*-c['\" ].*-o['\" ]" stdout \
          | $FGREP -v ' -c -o file.o' | wc -l) -eq 1
  # Once we have rewritten $CC to use our 'compile' wrapper script,
  # libtool should pick it up correctly, and not mess with the
  # redefinition.
  grep '^checking if .*/compile .*supports -c -o file\.o\.\.\. yes' stdout
  # And of course, we should be able to build our package.
  $MAKE
  cd $srcdir
done

$MAKE distcheck

:
