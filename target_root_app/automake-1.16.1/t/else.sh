#! /bin/sh
# Copyright (C) 1998-2018 Free Software Foundation, Inc.
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

# Test to make sure line numbers are correct in some error reports.

. test-init.sh

echo 'AM_CONDITIONAL(FOO, true)' >> configure.ac

cat > Makefile.am << 'END'
# Flag to tell us if apache dir is a source distribution.
APACHE_DIR_IS_SRC = @APACHE_DIR_IS_SRC@

# We only need to descend into the c dir if we're doing a 1.3 DSO
# configuration.
ifeq ("${APACHE_DIR_IS_SRC}","yes")
SUBDIRS = java
else
SUBDIRS = c java
endif
END

$ACLOCAL
AUTOMAKE_fails
grep '^Makefile\.am:8:.*else without if' stderr
