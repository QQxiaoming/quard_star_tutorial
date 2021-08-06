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

# Check that the global testsuite log file referenced in the testsuite
# summary and in the global testsuite log itself is correct.

. test-init.sh

mv configure.ac configure.stub

cat > fail << 'END'
#!/bin/sh
exit 1
END
chmod a+x fail

cat configure.stub - > configure.ac <<'END'
AC_OUTPUT
END

cat > Makefile.am << 'END'
TEST_SUITE_LOG = my_test_suite.log
TESTS = fail
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

mkdir build
cd build

../configure

run_make -O -e FAIL check
grep '^See \./my_test_suite\.log$' stdout

mkdir bar
run_make -O -e FAIL TEST_SUITE_LOG=bar/bar.log check
grep '^See \./bar/bar\.log$' stdout

cd ..

echo SUBDIRS = sub > Makefile.am
mkdir sub
echo TESTS = fail > sub/Makefile.am
mv fail sub

cat configure.stub - > configure.ac <<'END'
AC_CONFIG_FILES([sub/Makefile])
AC_OUTPUT
END

$ACLOCAL --force
$AUTOCONF --force
$AUTOMAKE

./configure
run_make -O -e FAIL check
grep '^See sub/test-suite\.log$' stdout
cd sub
run_make -O -e FAIL check
grep '^See sub/test-suite\.log$' stdout
cd ..

run_make -O -e FAIL TEST_SUITE_LOG=foo.log check
grep '^See sub/foo\.log$' stdout

:
