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

# Check that remake rules works from subdirectories, even using
# 'GNUmakefile' as makefiles name.  This obviously requires GNU
# make.
# See also the other similar tests 'remake-subdir*.sh', and the
# related test 'aclocal5.sh'

required=GNUmake
. test-init.sh

magic1='::MagicString::One::'
magic2='__MagicString__Two__'

debug_info ()
{
  grep -i magic configure GNUmakefile.in GNUmakefile \
                sub/GNUmakefile.in sub/GNUmakefile
}

cat > configure.ac <<END
AC_INIT([$me], [1.0])
AM_INIT_AUTOMAKE
AC_CONFIG_FILES([GNUmakefile sub/GNUmakefile])
AC_SUBST([MAGIC], [magic])
AC_OUTPUT
END

cat > GNUmakefile.am <<'END'
SUBDIRS = sub
END

mkdir sub
: > sub/GNUmakefile.am

$ACLOCAL
$AUTOCONF
$AUTOMAKE

./configure
$MAKE
debug_info

$sleep
sed "s|magic|$magic1|" configure.ac > t
mv -f t configure.ac
cd sub
$MAKE
cd ..
debug_info
$FGREP $magic1 configure
$FGREP $magic1 GNUmakefile
$FGREP $magic1 sub/GNUmakefile

$sleep
cd sub
echo MAGIC = $magic2 >> GNUmakefile.am
$MAKE
cd ..
debug_info
$FGREP $magic2 sub/GNUmakefile
$FGREP $magic2 sub/GNUmakefile.in
$FGREP $magic1 sub/GNUmakefile sub/GNUmakefile.in && exit 1
$FGREP $magic2 GNUmakefile GNUmakefile.in && exit 1

:
