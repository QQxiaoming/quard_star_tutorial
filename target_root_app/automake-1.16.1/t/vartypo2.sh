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

# Make sure we warn about possible variable typos when we should,
# Libtool variant.

required=libtoolize
. test-init.sh

cat >>configure.ac <<'END'
AM_PROG_AR
AC_PROG_LIBTOOL
AC_OUTPUT
END

cat >Makefile.am <<'END'
libfoo_la_SOURCES = unused
nodist_libfoo_la_SOURCES = unused
EXTRA_libfoo_la_SOURCES = unused
libfoo_la_LIBADD = unused
libfoo_la_DEPENDENCIES = unused
EXTRA_libfoo_la_DEPENDENCIES = unused
END

libtoolize
$ACLOCAL
AUTOMAKE_fails --add-missing
# The expected diagnostic is:
# automake: warnings are treated as errors
# Makefile.am:3: warning: variable 'EXTRA_libfoo_la_SOURCES' is defined but no program or
# Makefile.am:3: library has 'libfoo_la' as canonical name (possible typo)
# Makefile.am:1: warning: variable 'libfoo_la_SOURCES' is defined but no program or
# Makefile.am:1: library has 'libfoo_la' as canonical name (possible typo)
# Makefile.am:2: warning: variable 'nodist_libfoo_la_SOURCES' is defined but no program or
# Makefile.am:2: library has 'libfoo_la' as canonical name (possible typo)
# Makefile.am:4: warning: variable 'libfoo_la_LIBADD' is defined but no program or
# Makefile.am:4: library has 'libfoo_la' as canonical name (possible typo)
# Makefile.am:6: warning: variable 'EXTRA_libfoo_la_DEPENDENCIES' is defined but no program or
# Makefile.am:6: library has 'libfoo_la' as canonical name (possible typo)
# Makefile.am:5: warning: variable 'libfoo_la_DEPENDENCIES' is defined but no program or
# Makefile.am:5: library has 'libfoo_la' as canonical name (possible typo)

grep 'as canonical' stderr | grep -v ' .libfoo_la. ' && exit 1
test $(grep -c 'variable.*is defined but' stderr) -eq 6

# If we add a global -Wnone, all warnings should disappear.
$AUTOMAKE -Wnone

# Likewise, if matching programs or libraries are defined.
cat >>Makefile.am <<'END'
lib_LTLIBRARIES = libfoo.la
END

$AUTOMAKE

:
