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

# Automatic dependency tracking with subdir-objects option active:
# check for a pathological case of slash-collapsing in the name of
# included makefile fragments (containing dependency info).
# See also related tests 'subobj11a.sh' and 'subobj11b.sh'.

. test-init.sh

echo AC_PROG_CC >> configure.ac

cat > Makefile.am << 'END'
AUTOMAKE_OPTIONS = subdir-objects
bin_PROGRAMS = foo
foo_SOURCES = //zardoz.c
END

$ACLOCAL
$AUTOMAKE -a

#
# This check depends on automake internals, but presently this is
# the only way to test the code path we are interested in.
# Please update these checks when (and if) the relevant automake
# internals are changed.
#
# Be a little lax in the regexp, to account for automake conditionals,
# quoting, and similar stuff.
#
# FIXME: Are we sure this is the most sensible output in our situation?
#
grep 'am__depfiles_remade =.* //\$(DEPDIR)/zardoz\.Po' Makefile.in

:
