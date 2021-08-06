#! /bin/sh
# Copyright (C) 1998-2018 Free Software Foundation, Inc.
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

# Another test for a failing ":" in AC_OUTPUT.
# Report from Maciej Stachowiak.

. test-init.sh

cat > configure.ac << 'END'
AC_INIT([colon7], [1.0])
AM_INIT_AUTOMAKE
dnl: Please do not add proper m4 quoting here.
AC_OUTPUT(subdir/bar:subdir/foo \
   Makefile \
   subdir/Makefile
)
END

mkdir subdir
: > Makefile.am
: > subdir/foo
cat > subdir/Makefile.am << 'END'
# DIST_COMMON should contain 'foo', not 'subdir/foo'.
test:
	case '$(DIST_COMMON)' in *subdir/foo*) exit 1;; *) exit 0;; esac
	echo ' ' $(DIST_COMMON) ' ' | grep '[ /]foo '
.PHONY: test
END

$ACLOCAL
$AUTOMAKE

# Should nowhere refer to 'bar.in'.
$FGREP 'bar.in' Makefile.in subdir/Makefile.in && exit 1

$AUTOCONF
./configure
cd subdir
$MAKE test

:
