#! /bin/sh
# Copyright (C) 2012-2018 Free Software Foundation, Inc.
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

# Check that user recursion can be made to work even when $(SUBDIRS)
# is empty or undefined.

. test-init.sh

cat >> configure.ac <<'END'
AM_EXTRA_RECURSIVE_TARGETS([foo])
AC_OUTPUT
END

$ACLOCAL
$AUTOCONF

# First try with undefined $(SUBDIRS).
cat > Makefile.am <<'END'
all-local: foo
foo-local:
	: > bar
MOSTLYCLEANFILES = bar
END
$AUTOMAKE
./configure
$MAKE foo
test -f bar

$MAKE distclean
test ! -r bar # Sanity check.

# Now try with empty but defined $(SUBDIRS).
echo SUBDIRS = >> Makefile.am
$AUTOMAKE
./configure
$MAKE foo
test -f bar

:
