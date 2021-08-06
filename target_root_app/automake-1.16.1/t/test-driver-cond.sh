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

# parallel-tests:
#   - Automake can correctly handle conditionals contents for the
#     LOG_DRIVER variables.

. test-init.sh

cp "$am_testaux_srcdir"/trivial-test-driver . \
  || fatal_ "failed to fetch auxiliary script trivial-test-driver"
cp "$am_scriptdir"/tap-driver.sh . \
  || fatal_ "failed to fetch auxiliary script tap-driver.sh"

cat >> configure.ac << END
AM_CONDITIONAL([COND1], [:])
AM_CONDITIONAL([COND2], [false])
AM_CONDITIONAL([COND3], [false])
AC_SUBST([PERL], ['$PERL'])
AC_SUBST([my_LOG_DRIVER], ['\${SHELL} \${top_srcdir}/trivial-test-driver'])
AM_SUBST_NOTMAKE([my_LOG_DRIVER])
AC_OUTPUT
END

$ACLOCAL
$AUTOCONF

cat > Makefile.am << 'END'
TESTS = foo bar.test baz.sh
EXTRA_DIST = $(TESTS) tap-driver.sh trivial-test-driver
TEST_EXTENSIONS = .test .sh
LOG_DRIVER =
SH_LOG_DRIVER = $(tap_rulez)
if COND1
LOG_DRIVER += @my_LOG_DRIVER@
if COND2
tap_rulez = false
else !COND2
tap_rulez = $(PERL) $(srcdir)/tap-driver.sh
endif !COND2
endif COND1
END

cat > foo <<'END'
#!/bin/sh
echo "PASS: from $0"
echo "SKIP: from $0"
exit 1 # exit status should be ignored by the trivial-test-driver.
END

cat > bar.test <<'END'
#!/bin/sh
exit 0
END

cat > baz.sh <<'END'
#!/bin/sh
echo 1..3
echo 'ok 1'
echo 'not ok 2 # TODO'
echo 'not ok 3 # TODO'
END

chmod a+x foo bar.test baz.sh

$AUTOMAKE -a
test -f test-driver

grep DRIVER Makefile.in || exit 99 # For debugging.

grep '^my_LOG_DRIVER *=' Makefile.in \
  && fatal_ 'unexpected $(my_LOG_DRIVER) in Makefile.in'

grep '^TEST_LOG_DRIVER =.*\$(SHELL).*/test-driver' Makefile.in

$PERL -MTAP::Parser -e 1 \
  || skip_ "cannot import TAP::Parser perl module"

./configure

do_count ()
{
  count_test_results total=6 pass=3 fail=0 skip=1 xfail=2 xpass=0 error=0
  grep '^PASS: foo, testcase 1 *$' stdout
  grep '^SKIP: foo, testcase 2 *$' stdout
  grep '^PASS: bar\.test *$' stdout
  $EGREP 'PASS: baz\.sh 1( |$)' stdout
  $EGREP 'XFAIL: baz\.sh 2( |$)' stdout
  $EGREP 'XFAIL: baz\.sh 3( |$)' stdout
}

run_make -O -e IGNORE check
cat test-suite.log
cat foo.log
cat bar.log
cat baz.log
test $am_make_rc -eq 0 || exit 1
do_count

run_make -O distcheck
do_count

:
