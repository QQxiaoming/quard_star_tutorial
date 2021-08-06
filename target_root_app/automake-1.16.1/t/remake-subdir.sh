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

# Check that remake rules works from subdirectories, even with non-GNU
# make implementations.
# See also the other similar tests 'remake-subdir*.sh', and the
# related test 'aclocal5.sh'.

. test-init.sh

if using_gmake; then
  remake=$MAKE
else
  remake="$MAKE Makefile"
fi

magic1='::MagicString::One::'
magic2='__MagicString__Two__'

debug_info ()
{
  grep -i magic configure Makefile.in Makefile sub/Makefile.in sub/Makefile
}

cat >> configure.ac <<'END'
AC_CONFIG_FILES([sub/Makefile])
AC_SUBST([MAGIC], [magic])
AC_OUTPUT
END

cat > Makefile.am <<'END'
SUBDIRS = sub
END

mkdir sub
: > sub/Makefile.am

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
$remake
cd ..
debug_info
$FGREP $magic1 configure
$FGREP $magic1 Makefile
$FGREP $magic1 sub/Makefile

$sleep
cd sub
echo MAGIC = $magic2 >> Makefile.am
$remake
cd ..
debug_info
$FGREP $magic2 sub/Makefile
$FGREP $magic2 sub/Makefile.in
$FGREP $magic1 sub/Makefile sub/Makefile.in && exit 1
$FGREP $magic2 Makefile Makefile.in && exit 1

:
