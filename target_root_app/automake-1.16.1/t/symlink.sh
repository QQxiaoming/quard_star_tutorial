#! /bin/sh
# Copyright (C) 1996-2018 Free Software Foundation, Inc.
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

# Make sure we get an error if symlink creation fails.
# Reported by Joerg-Martin Schwarz.

am_create_testdir=empty
. test-init.sh

: > Makefile.am

cat > configure.ac << 'END'
AC_INIT([symlink], [1.0])
AC_CONFIG_AUX_DIR([subdir])
AM_INIT_AUTOMAKE
AC_CONFIG_FILES([Makefile])
END

$ACLOCAL
AUTOMAKE_fails --add-missing

grep '^configure\.ac:3: .*missing.*error while making link' stderr
grep '^configure\.ac:3: .*install-sh.*error while making link' stderr

test $(grep -c 'error while making link' stderr) -eq 2

:
