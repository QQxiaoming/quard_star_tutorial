#! /bin/sh
# Copyright (C) 2002-2018 Free Software Foundation, Inc.
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

# Make sure that man pages listed in man_MANS are installed and
# renamed as documented.

. test-init.sh

cat >> configure.ac <<'END'
AC_OUTPUT
END

cat > Makefile.am << 'END'
man_MANS = foo.2
man4_MANS = foo.4 bar.man
END

: > foo.2
: > foo.4
: > bar.man

$ACLOCAL
$AUTOCONF
$AUTOMAKE

cwd=$(pwd) || fatal_ "getting current working directory"

# Let's play with $DESTDIR too, it shouldn't hurt.
./configure --prefix='' --mandir=/man
$MAKE DESTDIR="$cwd/_inst" install

test -f _inst/man/man2/foo.2
test -f _inst/man/man4/foo.4
test -f _inst/man/man4/bar.4

$MAKE DESTDIR="$cwd/_inst" uninstall

test ! -e _inst/man/man2/foo.2
test ! -e _inst/man/man4/foo.4
test ! -e _inst/man/man4/bar.4

:
