#!/bin/sh
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

# Check that we don't emit harmless but useless code in the generated
# Makefile.in when the project does not use compiled languages.  Motivated
# by a regression caused by removal of automatic de-ANSI-fication support:
# <https://lists.gnu.org/archive/html/automake-patches/2011-08/msg00200.html>

. test-init.sh

echo AC_OUTPUT >> configure.ac

: > Makefile.am

# Automake shouldn't need nor require these.
rm -f depcomp compile

$ACLOCAL

# Don't use '--add-missing' here, so that we can implicitly
# check that the auxiliary scripts 'compile' and 'depcomp'
# are not spuriously required by Automake.
$AUTOMAKE

$EGREP 'INCLUDES|@am__isrc@|-compile|\$\(OBJEXT\)|tab\.[ch]' \
  Makefile.in && exit 1

:
