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

# Check that Automake-generated C compilation rules don't mistakenly
# use the "-c -o" options combination unconditionally (even with losing
# compilers) when the 'subdir-objects' is used but sources are only
# present in the top-level directory.  Reported by Nick Bowler in the
# discussion on automake bug#13378:
# <https://debbugs.gnu.org/cgi/bugreport.cgi?bug=13378#35>
# <https://debbugs.gnu.org/cgi/bugreport.cgi?bug=13378#44>

required=gcc # For cc-no-c-o.
. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
$CC --version; $CC -v; # For debugging.
AC_OUTPUT
END

cat > Makefile.am << 'END'
AUTOMAKE_OPTIONS = subdir-objects
bin_PROGRAMS = foo bar
bar_SOURCES = foo.c
END

echo 'int main (void) { return 0; }' > foo.c

# Make sure the compiler doesn't understand '-c -o'
CC=$am_testaux_builddir/cc-no-c-o; export CC

$ACLOCAL
$AUTOCONF
$AUTOMAKE --copy --add-missing

./configure
$MAKE
$MAKE distcheck

:
