#! /bin/sh
# Copyright (C) 2003-2018 Free Software Foundation, Inc.
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

# Test to make sure that adding a new directory works, even from
# subdirectories.  The sister test 'subdir-add-pr46.sh' makes sure
# it works when make is run from the top-level directory.
# PR automake/46

. test-init.sh

cat >> configure.ac << 'END'
AC_CONFIG_MACRO_DIR([m4])
m4_include([confiles.m4])
MORE_DEFS
AC_OUTPUT
END

echo 'AC_CONFIG_FILES([sub/Makefile])' > confiles.m4

cat > Makefile.am << 'END'
SUBDIRS = sub
END

mkdir sub

: > sub/Makefile.am

mkdir m4
echo 'AC_DEFUN([MORE_DEFS], [])' > m4/moredefs.m4

$ACLOCAL
$AUTOCONF
$AUTOMAKE
./configure
$MAKE

# Now add new directories.

# The first step users typically do when adding a new subdir is editing
# configure.ac.  That is already tested by 'subdir-add-pr46.sh' though,
# so here we try to just edit a file that is included by configure.ac,
# without touching configure.ac itself.

mkdir sub/maude
cat > sub/maude/Makefile.am << 'END'
include_HEADERS = foo.h
END

: > sub/maude/foo.h

echo 'SUBDIRS = maude' >> sub/Makefile.am

mkdir maude
: > maude/Makefile.am

# Update confiles.m4 *after* updating sub/Makefile.am; the sister test
# 'subdir-add-pr46.sh' does it the in other way: it updates configure.ac
# before Makefile.am.  We sleep here because modified configure
# dependencies must be newer than config.status.
$sleep
echo 'AC_CONFIG_FILES([maude/Makefile sub/maude/Makefile])' >> confiles.m4

# We want a simple rebuild from sub/ to create sub/maude/Makefile
# and maude/Makefile automatically.
cd sub
$MAKE
cd ..
grep '^SUBDIRS = *maude *$' sub/Makefile.in
grep '^SUBDIRS = *maude *$' sub/Makefile
test -f maude/Makefile
test -f sub/maude/Makefile

# Make sure the dependencies of aclocal.m4 or honored at least from
# the top-level directory.
echo 'AC_DEFUN([MORE_DEFS], [AC_SUBST([GREPME])])' > m4/moredefs.m4
$MAKE

for ext in '.in' ''; do
  for d in . maude sub sub/maude; do
    grep '^GREPME =' $d/Makefile$ext
  done
done

:
