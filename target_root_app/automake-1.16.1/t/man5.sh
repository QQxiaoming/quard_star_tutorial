#! /bin/sh
# Copyright (C) 2009-2018 Free Software Foundation, Inc.
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

# Make sure to extract the correct mansection from files in man_MANS.

. test-init.sh

cat >> configure.ac <<'END'
AC_OUTPUT
END

cat > Makefile.am << 'END'
man_MANS = foo-1.4.5/foo.2 foo-1.4.5/bar.3 baz-1.4.2
notrans_man_MANS = foo-1.4.5/nfoo.2 foo-1.4.5/nbar.3 nbaz-1.4.2
END

mkdir foo-1.4.5

: > foo-1.4.5/foo.2
: > foo-1.4.5/nfoo.2
: > foo-1.4.5/bar.3
: > foo-1.4.5/nbar.3
: > baz-1.4.2
: > nbaz-1.4.2

$ACLOCAL
$AUTOCONF
$AUTOMAKE

cwd=$(pwd) || fatal_ "getting current working directory"

# Let's play with $DESTDIR too, it shouldn't hurt.
./configure --mandir=/man
$MAKE DESTDIR="$cwd/_inst" install

test -f _inst/man/man2/foo.2
test -f _inst/man/man2/nfoo.2
test -f _inst/man/man2/baz-1.4.2
test -f _inst/man/man2/nbaz-1.4.2
test -f _inst/man/man3/bar.3
test -f _inst/man/man3/nbar.3

test ! -e _inst/man/man1
test ! -e _inst/man/man4
test ! -e _inst/man/man5

$MAKE DESTDIR="$cwd/_inst" uninstall

test ! -e _inst/man/man2/foo.2
test ! -e _inst/man/man2/nfoo.2
test ! -e _inst/man/man2/baz-1.4.2
test ! -e _inst/man/man2/nbaz-1.4.2
test ! -e _inst/man/man3/bar.3
test ! -e _inst/man/man3/nbar.3

:
