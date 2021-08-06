#! /bin/sh
# Copyright (C) 1999-2018 Free Software Foundation, Inc.
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

# Test for another '+=' problem.  Report from Brian Jones.

. test-init.sh

cat >> configure.ac << 'END'
AM_CONDITIONAL([CHECK], [true])
END

cat > Makefile.am << 'END'
if CHECK
INCLUDES = abc
endif
INCLUDES += def
END

$ACLOCAL
AUTOMAKE_fails

# We expect the following diagnostic:
#
# Makefile.am:4: cannot apply '+=' because 'INCLUDES' is not defined in
# Makefile.am:4: the following conditions:
# Makefile.am:4:   !CHECK
# Makefile.am:4: either define 'INCLUDES' in these conditions, or use
# Makefile.am:4: '+=' in the same conditions as the definitions.

# Is !CHECK mentioned?
grep ':.*!CHECK$' stderr
# Is there only one missing condition?
test $(grep -c ':  ' stderr) -eq 1

# By the way, Automake should suggest using AM_CPPFLAGS,
# because INCLUDES is an obsolete name.
grep AM_CPPFLAGS stderr

# A way to suppress the obsolete warning is to use
# -Wno-obsolete:
echo 'AUTOMAKE_OPTIONS = -Wno-obsolete' >> Makefile.am
AUTOMAKE_fails
grep AM_CPPFLAGS stderr && exit 1
# !CHECK should still be mentioned.
grep ':.*!CHECK$' stderr

:
