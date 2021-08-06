#! /bin/sh
# Copyright (C) 2001-2018 Free Software Foundation, Inc.
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

# It is ok to have a conditional SUBDIRS when using gettext.

required=gettext
. test-init.sh

cat >> configure.ac << 'END'
AM_GNU_GETTEXT
AM_CONDITIONAL([MAUDE], [true])
ALL_LINGUAS=
AC_SUBST([ALL_LINGUAS])
END

mkdir po intl
: >config.rpath

cat > Makefile.am << 'END'
if MAUDE
SUBDIRS = po intl
else
SUBDIRS =
endif
END

$ACLOCAL
# Gettext wants config.guess etc.
$AUTOMAKE --add-missing

:
