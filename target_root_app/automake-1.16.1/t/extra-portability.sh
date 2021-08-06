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

# Check interactions between the 'portability' and 'extra-portability'
# warning categories:
#   1. '-Wextra-portability' must imply '-Wportability'.
#   2. '-Wno-portability' must imply '-Wno-extra-portability'.
#   3. '-Wall' must imply '-Wextra-portability'.

. test-init.sh

# We want (almost) complete control over automake options.
AUTOMAKE="$am_original_AUTOMAKE --foreign -Werror"

cat >>configure.ac <<END
AC_PROG_CC
AC_PROG_RANLIB
AC_OUTPUT
END

$ACLOCAL

#
# First, a setup where only an extra-portability warning is present
# (no "simple" portability-warnings are).
#

cat >Makefile.am <<END
EXTRA_LIBRARIES = libfoo.a
libfoo_a_SOURCES = foo.c
END

# Sanity check: extra-portability warnings causes the expected error.
AUTOMAKE_fails -Wextra-portability
grep 'requires.*AM_PROG_AR' stderr

# Warnings in extra-portability category are not enabled by default.
$AUTOMAKE

# -Wall enables extra-portability.
AUTOMAKE_fails -Wall
grep 'requires.*AM_PROG_AR' stderr

# Disabling portability disables extra-portability as well.
$AUTOMAKE -Wextra-portability -Wno-portability
$AUTOMAKE -Wall -Wno-portability

#
# Now, a setup where also a "simple" portability warning is present.
#

echo 'var = $(foo--bar)' >> Makefile.am

# Enabling extra-portability enables portability as well ...
AUTOMAKE_fails -Wextra-portability
grep 'foo--bar' stderr
grep 'requires.*AM_PROG_AR' stderr
# ... even if it had been previously disabled.
AUTOMAKE_fails -Wno-portability -Wextra-portability
grep 'foo--bar' stderr
grep 'requires.*AM_PROG_AR' stderr

# Disabling extra-portability leaves portability intact (1).
AUTOMAKE_fails -Wportability -Wno-extra-portability
grep 'foo--bar' stderr
grep 'requires.*AM_PROG_AR' stderr && exit 1
# Disabling extra-portability leaves portability intact (2).
AUTOMAKE_fails -Wall -Wno-extra-portability
grep 'foo--bar' stderr
grep 'requires.*AM_PROG_AR' stderr && exit 1

# Enabling portability does not enable extra-portability.
AUTOMAKE_fails -Wportability
grep 'foo--bar' stderr
grep 'requires.*AM_PROG_AR' stderr && exit 1

# Disabling portability disables extra-portability.
$AUTOMAKE -Wno-portability
$AUTOMAKE -Wextra-portability -Wno-portability
$AUTOMAKE -Wall -Wno-portability

:
