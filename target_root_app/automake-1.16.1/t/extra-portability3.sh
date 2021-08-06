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

# Check interactions between the 'portability-recursive' and
# 'extra-portability' warning categories.

. test-init.sh

# We want (almost) complete control over automake options.
AUTOMAKE="$am_original_AUTOMAKE --foreign -Werror"

cat >>configure.ac <<END
AC_PROG_CC
AC_PROG_RANLIB
AC_OUTPUT
END

$ACLOCAL

cat >Makefile.am <<'END'
baz = $(foo$(bar))
lib_LIBRARIES = libfoo.a
libfoo_a_SOURCES = foo.c
END

# 'extra-portability' implies 'portability-recursive'.
AUTOMAKE_fails -Wextra-portability
grep 'requires.*AM_PROG_AR' stderr
grep 'recursive variable expansion' stderr

# We can disable 'extra-portability' while leaving
# 'portability-recursive' intact.
AUTOMAKE_fails -Wportability-recursive -Wno-extra-portability
grep 'requires.*AM_PROG_AR' stderr && exit 1
grep 'recursive variable expansion' stderr

# We can disable 'portability-recursive' while leaving
# 'extra-portability' intact.
AUTOMAKE_fails -Wextra-portability -Wno-portability-recursive
grep 'requires.*AM_PROG_AR' stderr
grep 'recursive variable expansion' stderr && exit 1

# Disabling 'portability' disables 'portability-recursive' and
# 'extra-portability'.
$AUTOMAKE -Wextra-portability -Wno-portability

:
