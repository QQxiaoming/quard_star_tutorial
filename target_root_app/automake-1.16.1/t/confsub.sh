#! /bin/sh
# Copyright (C) 1996-2018 Free Software Foundation, Inc.
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

# Test to make sure config.h can be in subdir.
# Also, make sure config.h is properly rebuilt.

. test-init.sh

cat >> configure.ac << 'END'
AC_CONFIG_FILES([subdir/Makefile])
AC_CONFIG_HEADERS([subdir/config.h:subdir/config.hin])
AC_OUTPUT
END

cat > Makefile.am << 'END'
SUBDIRS = subdir
END

mkdir subdir
cat > subdir/Makefile.am << 'END'
END

echo 'Before.' > subdir/config.hin

$ACLOCAL
$AUTOMAKE

# Make sure subdir Makefile.in doesn't itself look in the subdir.
# One line is allowed though:
#
# cd $(top_builddir) && $(SHELL) ./config.status subdir/config.h
($FGREP 'subdir/config.h' subdir/Makefile.in |
   $FGREP -v 'cd $(top_builddir)') && exit 1

$AUTOCONF
./configure
$FGREP 'Before.' subdir/config.h

$sleep
echo 'After.' > subdir/config.hin
$MAKE
$FGREP 'After.' subdir/config.h

:
