#! /bin/sh
# Copyright (C) 2009-2018 Free Software Foundation, Inc.
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

# Check parallel-tests features:
#  - check-html
#  - recheck-html

. test-init.sh

# Try the variants that are tried in check-html.am.
while :; do
  for r2h in $RST2HTML rst2html rst2html.py; do
    echo "$me: running $r2h --version"
    $r2h --version && break 2
    : For shells with busted 'set -e'.
  done
  skip_all_ "no proper rst2html program found"
done
unset r2h

cp "$am_top_srcdir"/contrib/check-html.am . \
  || fatal_ "cannot fetch 'check-html.am' from contrib"

cat >> configure.ac << 'END'
AC_OUTPUT
END

cat > Makefile.am << 'END'
TEST_SUITE_LOG = mylog.log
TESTS = foo.test bar.test baz.test
check_SCRIPTS = bla
bla:
	echo bla > $@
CLEANFILES = bla
include $(srcdir)/check-html.am
END

cat > foo.test <<'END'
#! /bin/sh
echo "this is $0"
test -f bla || exit 1
exit 0
END

cat > bar.test <<'END'
#! /bin/sh
echo "this is $0"
exit 99
END

cat > baz.test <<'END'
#! /bin/sh
echo "this is $0"
exit 1
END

chmod a+x foo.test bar.test baz.test

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure

$MAKE check-html && exit 1
test -f mylog.html
# check-html should cause check_SCRIPTS to be created.
test -f bla

# "make clean" should remove HTML files.
$MAKE clean
test ! -e mylog.html
test ! -e bla

# Always create the HTML output, even if there were no failures.
rm -f mylog.html
run_make TESTS=foo.test check-html
test -f mylog.html

# Create summarizing HTML output also with recheck-html.
rm -f mylog.html
run_make TESTS=foo.test recheck-html
test -f mylog.html

# Create HTML output for an individual test.
$MAKE foo.html
grep 'this is .*foo\.test' foo.html
test ! -e bar.html
test ! -e baz.html

# Create HTML output for individual tests.  Since the pre-existing log
# files are expected to be used for the HTML conversion, this should
# go smoothly even for failed tests.
$MAKE bar.html baz.html
grep 'this is .*bar\.test' bar.html
grep 'this is .*baz\.test' baz.html

# HTML output removed by mostlyclean.
$MAKE mostlyclean
test ! -e foo.html
test ! -e bar.html
test ! -e baz.html
test ! -e mylog.html

# check-html and recheck-html should cause check_SCRIPTS to be created,
# and recheck-html should rerun no tests if check has not been run.

$MAKE clean
test ! -e mylog.html
run_make TEST_LOGS=foo.log check-html
test -f bla
test -f foo.log
test ! -e bar.log
test ! -e baz.log
test -f mylog.html

$MAKE clean
run_make TESTS=foo.test recheck-html
test -f bla
test ! -e foo.log
test -f mylog.html

$MAKE clean
$MAKE recheck-html
test -f bla
test ! -e foo.log
test ! -e bar.log
test ! -e baz.log
test -f mylog.html

:
