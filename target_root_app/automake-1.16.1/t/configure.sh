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

# Diagnose if the autoconf input is named configure.in.
# Diagnose if both configure.in and configure.ac are present, prefer
# configure.ac.

. test-init.sh

cat >configure.ac <<EOF
AC_INIT([$me], [1.0])
AM_INIT_AUTOMAKE
AC_CONFIG_FILES([Makefile])
EOF

cat >configure.in <<EOF
AC_INIT([$me], [1.0])
AM_INIT_AUTOMAKE([an-invalid-automake-option])
AC_CONFIG_FILES([Makefile])
EOF

: >Makefile.am

$ACLOCAL 2>stderr && { cat stderr >&2; exit 1; }
cat stderr >&2
grep 'configure\.ac.*configure\.in.*both present' stderr

$ACLOCAL -Wno-error 2>stderr || { cat stderr >&2; exit 1; }
cat stderr >&2
grep 'configure\.ac.*configure\.in.*both present' stderr
grep 'proceeding.*configure\.ac' stderr

# Ensure we really proceed with configure.ac.
AUTOMAKE_fails -Werror
grep 'configure\.ac.*configure\.in.*both present' stderr
grep 'proceeding.*configure\.ac' stderr

AUTOMAKE_run -Wno-error
grep 'configure\.ac.*configure\.in.*both present' stderr
grep 'proceeding.*configure\.ac' stderr

mv -f configure.ac configure.in
AUTOMAKE_fails
grep "autoconf input.*'configure.ac', not 'configure.in'" stderr

:
