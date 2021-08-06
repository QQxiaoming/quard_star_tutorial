#! /bin/sh
# Copyright (C) 2008-2018 Free Software Foundation, Inc.
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

# Make sure remaking rules work when subdir Makefile.in has been removed.

. test-init.sh

cat >> configure.ac << 'END'
AC_CONFIG_FILES([sub/Makefile])
AC_OUTPUT
END

cat > Makefile.am <<'END'
SUBDIRS = sub
END
mkdir sub
: > sub/Makefile.am

$ACLOCAL
$AUTOMAKE
$AUTOCONF
./configure
$MAKE

do_check ()
{
  run_make -O
  test $(grep -c "/missing " stdout) -eq 1
}

# Now, we are set up.  Ensure that, for either missing Makefile.in,
# or updated Makefile.am, rebuild rules are run, and run exactly once
# only.

rm -f Makefile.in
do_check

rm -f sub/Makefile.in
do_check

$sleep  # Let touched files appear newer.

touch Makefile.am
do_check

touch sub/Makefile.am
do_check

:
