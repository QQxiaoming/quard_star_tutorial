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

# Check that explicitly-defined warning levels take precedence over
# implicit strictness-implied warnings, even when these explicit warning
# levels are "catch-all warnings" (such as '-Wall' and '-Wnone').
# Since we are at it, also throw in *.am and *.m4 file inclusions.

. test-init.sh

# We want complete control over automake options.
AUTOMAKE=$am_original_AUTOMAKE

# Files required in gnu and/or gnits strictness.
touch README INSTALL NEWS AUTHORS ChangeLog COPYING THANKS

cat > configure.ac << END
AC_INIT([$me], [1.0])
m4_include([am-init-automake.m4])
AC_PROG_CC
AC_CONFIG_FILES([Makefile])
# Other similar tests do not use AC_OUTPUT, so we use it here,
# for completeness and for better coverage.
AC_OUTPUT
END

cat > Makefile.am <<END
include automake-options.am
FOO := bar
END

rm -rf autom4te*.cache
: > automake-options.am
echo 'AM_INIT_AUTOMAKE' > am-init-automake.m4
$ACLOCAL
AUTOMAKE_fails -Werror -Wall --foreign
grep '^Makefile\.am:.*:=.*not portable' stderr

rm -rf autom4te*.cache
: > automake-options.am
echo 'AM_INIT_AUTOMAKE([-Werror -Wnone gnu])' > am-init-automake.m4
$ACLOCAL
$AUTOMAKE

rm -rf autom4te*.cache
echo 'AUTOMAKE_OPTIONS = -Werror -Wnone gnits' > automake-options.am
echo 'AM_INIT_AUTOMAKE' > am-init-automake.m4
$ACLOCAL
$AUTOMAKE

:
