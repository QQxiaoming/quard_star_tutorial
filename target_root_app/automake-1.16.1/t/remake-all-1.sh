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

# Check that the "all" target triggers rebuilt of outdated Makefiles.
# See also sister test 'remake-all-2.sh'.

. test-init.sh

mkfile=Makefile

magic1=::MagicStringOne::
magic2=__MagicStringTwo__
magic3=%%MagicStringThree%%

cat > configure.ac <<END
AC_INIT([$me], [1.0])
AM_INIT_AUTOMAKE
AC_CONFIG_FILES([$mkfile sub/$mkfile])
FOO='$magic1'
AC_OUTPUT
END

mkdir sub
echo SUBDIRS = sub > $mkfile.am
: > sub/$mkfile.am

$ACLOCAL
$AUTOMAKE

$EGREP "^all-am:.* $mkfile( |$)" $mkfile.in sub/$mkfile.in

$AUTOCONF
./configure

$FGREP "$magic1" mkfile.in && exit 1 # Sanity check.

# Two code paths in configure.am:

# - either a file in $(am__configure_deps) has been updated ...
$sleep
echo "AC_SUBST([FOO])" >> configure.ac
$MAKE
$FGREP FOO $mkfile # For debugging.
$FGREP $magic1 $mkfile

# - ... or not; i.e., Makefile.am or an included file has.
$sleep
echo "# $magic2" >> $mkfile.am
echo "# $magic3" >> sub/$mkfile.am
$MAKE
$FGREP $magic2 $mkfile
$FGREP $magic3 sub/$mkfile

:
