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

# Test to make sure that adding a new directory works.
# This test runs 'make' from the top-level directory, the sister
# test 'subdir-add2-pr46.sh' do it from a subdirectory.
# PR automake/46

. test-init.sh

cat >> configure.ac << 'END'
AC_OUTPUT
END

: > Makefile.am

$ACLOCAL
$AUTOCONF
$AUTOMAKE
./configure
$MAKE

# Now add new directories.

# First we add a new directory by modifying configure.ac directly.
# We update configure.ac *before* updating sub/Makefile.am; the sister
# test 'subdir-add2-pr46.sh' does it in the other way: it updates
# confiles.m4 (which is m4_included by configure.ac there) after
# Makefile.am.

# Modified configure dependencies must be newer than config.status.
$sleep
sed <configure.ac >configure.tmp -e '/^AC_OUTPUT$/i\
AC_CONFIG_FILES([maude/Makefile])\
m4_include([confile.m4])\
' # Last newline required by older OpenBSD sed.
mv -f configure.tmp configure.ac

cat configure.ac # For debugging.

: > confile.m4

mkdir maude

cat > maude/Makefile.am << 'END'
include_HEADERS = foo.h
END

: > maude/foo.h

echo 'SUBDIRS = maude' >> Makefile.am

# We want a simple rebuild to create maude/Makefile automatically.
$MAKE
grep '^SUBDIRS = *maude *$' Makefile.in
grep '^SUBDIRS = *maude *$' Makefile
test -f maude/Makefile

# Then we add a new directory by modifying a file included (through
# 'm4_include') by configure.ac.
mkdir maude2
# Modified configure dependencies must be newer than config.status.
$sleep
cat >> confile.m4 << 'END'
AC_CONFIG_FILES([maude2/Makefile])
AC_SUBST([GREPME])
END
: > maude2/Makefile.am
echo 'SUBDIRS += maude2' >> Makefile.am

# We want a simple rebuild to create maude2/Makefile and update
# all other Makefiles automatically.
$MAKE
grep '^SUBDIRS =.* maude2' Makefile.in
grep '^SUBDIRS =.* maude2' Makefile

for ext in '.in' ''; do
  for d in . maude maude2; do
    grep '^GREPME =' $d/Makefile$ext
  done
done

:
