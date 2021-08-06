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

# Option 'serial-tests'.

am_create_testdir=empty
. test-init.sh

hasnt_parallel_tests ()
{
  $EGREP 'TEST_SUITE_LOG|TEST_LOGS|\.log.*:' $1 && exit 1
  grep 'recheck.*:' $1 && exit 1
  grep '^check-TESTS: \$(TESTS)$' $1
}

has_parallel_tests ()
{
  $EGREP '(^| )check-TESTS.*:' $1
  $EGREP '(^| )recheck.*:' $1
  grep '^\$(TEST_SUITE_LOG): \$(TEST_LOGS)$' $1
  grep '^\.test\.log:$' $1
}

mkdir one two

cat > one/configure.ac <<END
AC_INIT([$me], [1.0])
AM_INIT_AUTOMAKE([serial-tests])
AC_CONFIG_FILES([Makefile])
END

echo 'TESTS = foo.test bar.test' > one/Makefile.am

cat > two/configure.ac <<END
AC_INIT([$me], [2.0])
AC_CONFIG_AUX_DIR([config])
AM_INIT_AUTOMAKE([parallel-tests])
AC_CONFIG_FILES([aMakefile bMakefile])
END

cp one/Makefile.am two/aMakefile.am
cat - one/Makefile.am > two/bMakefile.am <<END
AUTOMAKE_OPTIONS = serial-tests
END

cd one
touch missing install-sh
$ACLOCAL
$AUTOMAKE
grep TEST Makefile.in # For debugging.
hasnt_parallel_tests Makefile.in
test ! -e test-driver
cd ..

cd two
mkdir config
$ACLOCAL
$AUTOMAKE --add-missing
grep TEST [ab]Makefile.in # For debugging.
has_parallel_tests aMakefile.in
hasnt_parallel_tests bMakefile.in
mv aMakefile.in aMakefile.sav
mv bMakefile.in bMakefile.sav
test ! -e test-driver
test -f config/test-driver
$AUTOMAKE
diff aMakefile.sav aMakefile.in
diff bMakefile.sav bMakefile.in
cd ..

:
