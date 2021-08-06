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

# Check that auxiliary script 'test-driver' gets automatically installed
# in the correct directory.

am_create_testdir=empty
. test-init.sh

cat > configure.ac <<END
AC_INIT([$me], [1.0])
AC_CONFIG_AUX_DIR([my_aux_dir])
AM_INIT_AUTOMAKE
AC_CONFIG_FILES([Makefile sub/Makefile])
AC_OUTPUT
END

mkdir sub my_aux_dir

cat > Makefile.am <<END
SUBDIRS = sub
TESTS = foo.test
END

cat > sub/Makefile.am <<END
TESTS = bar.test
END

$ACLOCAL
$AUTOMAKE -a 2>stderr || { cat stderr >&2; exit 1; }
cat stderr >&2

ls -l . sub my_aux_dir # For debugging.
test -f my_aux_dir/test-driver
test ! -e test-driver
test ! -e sub/test-driver

grep '^parallel-tests:.*installing.*my_aux_dir/test-driver' stderr

:
