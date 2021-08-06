#! /bin/sh
# Copyright (C) 2012-2018 Free Software Foundation, Inc.
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

# Check against automake bug#10997: directories holding only
# conditionally-installed files should not be created unconditionally.
# FIXME: this test tries to check that the above also hold for the
# FIXME: "installdirs" target, but that doesn't currently work :-(
# FIXME: So this test is still xfailing.
# See sister test 'instdir-cond.sh' for the succeeding part.

. test-init.sh

cat >> configure.ac << 'END'
AM_CONDITIONAL([ENABLE_FOO], [false])
AC_OUTPUT
END

: > foo
: > bar

cat > Makefile.am << 'END'
pkglibexec_SCRIPTS =
if ENABLE_FOO
pkgdata_DATA = foo
pkglibexec_SCRIPTS += bar
endif
END

$ACLOCAL
$AUTOMAKE
$AUTOCONF

./configure --prefix="$(pwd)/inst"

$MAKE installdirs
test ! -e inst || { find inst; exit 1; }

:
