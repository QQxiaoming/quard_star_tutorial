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

# Check gettext 'external' support.
# PR/338, reported by Charles Wilson.

required='gettext'
. test-init.sh

cat >>configure.ac <<END
AM_GNU_GETTEXT([external])
AC_OUTPUT
END

: >Makefile.am
mkdir foo po

$ACLOCAL
$AUTOCONF

# config.rpath is required.
: >config.rpath

# po/ is required, but intl/ isn't.

AUTOMAKE_fails --add-missing
grep 'AM_GNU_GETTEXT.*SUBDIRS' stderr

echo 'SUBDIRS = foo' >Makefile.am
AUTOMAKE_fails --add-missing
grep 'AM_GNU_GETTEXT.*po' stderr

# Ok.

echo 'SUBDIRS = po' >Makefile.am
$AUTOMAKE --add-missing


# Don't try running ./configure --with-included-gettext if the
# user is using AM_GNU_GETTEXT([external]).
grep 'with-included-gettext' Makefile.in && exit 1
./configure
$MAKE -n distcheck | grep 'with-included-gettext' && exit 1

# intl/ isn't wanted with AM_GNU_GETTEXT([external]).

mkdir intl
echo 'SUBDIRS = po intl' >Makefile.am
AUTOMAKE_fails --add-missing
grep 'intl.*AM_GNU_GETTEXT' stderr

:
