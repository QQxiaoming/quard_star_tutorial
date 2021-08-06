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

# Check that remake rules works from subdirectories, even when makefiles
# are not named "Makefile".
# See also the other similar tests 'remake-subdir*.sh', and the
# related test 'aclocal5.sh'.

. test-init.sh

magic1='::MagicString::One::'
magic2='__MagicString__Two__'

debug_info ()
{
  grep -i magic configure build.in build.mk sub/build.in sub/build.mk
}

cat > configure.ac <<END
AC_INIT([$me], [1.0])
AM_INIT_AUTOMAKE
AC_CONFIG_FILES([build.mk:build.in])
AC_CONFIG_FILES([sub/build.mk:sub/build.in])
AC_SUBST([MAGIC], [magic])
AC_OUTPUT
END

cat > build.am <<'END'
AM_MAKEFLAGS = -f build.mk
SUBDIRS = sub
END

mkdir sub
cat > sub/build.am <<'END'
AM_MAKEFLAGS = -f build.mk
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE
./configure
ls -l # For debugging.

$MAKE -f build.mk
debug_info

$sleep
sed "s|magic|$magic1|" configure.ac > t
mv -f t configure.ac
cd sub
$MAKE -f build.mk build.mk
cd ..
debug_info
$FGREP $magic1 configure
$FGREP $magic1 build.mk
$FGREP $magic1 sub/build.mk

$sleep
cd sub
echo MAGIC = $magic2 >> build.am
$MAKE -f build.mk build.mk
cd ..
debug_info
$FGREP $magic2 sub/build.mk
$FGREP $magic2 sub/build.in
$FGREP $magic1 sub/build.in sub/build.mk && exit 1
$FGREP $magic2 build.in build.mk && exit 1

:
