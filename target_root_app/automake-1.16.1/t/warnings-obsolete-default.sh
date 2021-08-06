#! /bin/sh
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

# Check that automake warnings in the 'obsolete' category are enabled
# by default.

. test-init.sh

# We want (almost) complete control over automake options.
AUTOMAKE="$am_original_AUTOMAKE --foreign -Werror"

echo AC_PROG_CC >> configure.ac

cat > Makefile.am <<'END'
bin_PROGRAMS = foo
INCLUDES = -Ibar
END

$ACLOCAL
AUTOMAKE_fails
grep '^Makefile\.am:2:.*INCLUDES.*AM_CPPFLAGS' stderr

# Check that we can override warnings about obsolete stuff.
$AUTOMAKE -Wno-obsolete

:
