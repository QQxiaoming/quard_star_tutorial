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

# Check that an example given in the documentation really works.
# See section "Using the TAP test protocol", subsection "Use TAP
# with the Automake test harness".

am_create_testdir=empty
. test-init.sh

cat > Makefile.am <<'END'
TEST_LOG_DRIVER = env AM_TAP_AWK='$(AWK)' $(SHELL) \
                  $(top_srcdir)/build-aux/tap-driver.sh
TESTS = foo.test bar.test baz.test
EXTRA_DIST = $(TESTS)
END

cat > configure.ac <<'END'
AC_INIT([GNU Try Tap], [1.0], [bug-automake@gnu.org])
AC_CONFIG_AUX_DIR([build-aux])
AM_INIT_AUTOMAKE([foreign -Wall -Werror])
AC_CONFIG_FILES([Makefile])
AC_REQUIRE_AUX_FILE([tap-driver.sh])
AC_OUTPUT
END

cat > foo.test <<'END'
#!/bin/sh
echo 1..4 # Number of tests to be executed.
echo 'ok 1 - Swallows fly'
echo 'not ok 2 - Caterpillars fly # TODO metamorphosis in progress'
echo 'ok 3 - Pigs fly # SKIP not enough acid'
echo '# I just love word plays...'
echo 'ok 4 - Flies fly too :-)'
END

cat > bar.test <<'END'
#!/bin/sh
echo 1..3
echo 'not ok 1 - Bummer, this test has failed.'
echo 'ok 2 - This passed though.'
echo 'Bail out! Ennui kicking in, sorry...'
echo 'ok 3 - This will not be seen.'
END

cat > baz.test <<'END'
#!/bin/sh
echo 1..1
echo ok 1
# Exit with error, even if all the tests have been successful.
exit 7
END

chmod a+x *.test

# Strip extra "informative" lines that could be printed by Solaris
# Distributed Make.
mkdir build-aux
cp "$am_scriptdir"/tap-driver.sh build-aux \
  || framework_failure_ "fetching the perl TAP driver"

(export AUTOMAKE ACLOCAL AUTOCONF && $AUTORECONF -vi) || exit 1

./configure --help # Sanity check.
./configure || skip_ "configure failed"

case $MAKE in *\ -j*) skip_ "can't work easily with concurrent make";; esac

# Prevent Sun Distributed Make from trying to run in parallel.
DMAKE_MODE=serial; export DMAKE_MODE

run_make -O -e FAIL check

cat > exp <<'END'
PASS: foo.test 1 - Swallows fly
XFAIL: foo.test 2 - Caterpillars fly # TODO metamorphosis in progress
SKIP: foo.test 3 - Pigs fly # SKIP not enough acid
PASS: foo.test 4 - Flies fly too :-)
FAIL: bar.test 1 - Bummer, this test has failed.
PASS: bar.test 2 - This passed though.
ERROR: bar.test - Bail out! Ennui kicking in, sorry...
PASS: baz.test 1
ERROR: baz.test - exited with status 7
END

sed -n '/^PASS: foo\.test/,/^ERROR: baz\.test/p' stdout > got

cat exp
cat got
diff exp got

grep '^Please report to bug-automake@gnu\.org$' stdout

run_make -O check \
  TESTS='foo.test baz.test' \
  TEST_LOG_DRIVER_FLAGS='--comments --ignore-exit'

cat > exp <<'END'
PASS: foo.test 1 - Swallows fly
XFAIL: foo.test 2 - Caterpillars fly # TODO metamorphosis in progress
SKIP: foo.test 3 - Pigs fly # SKIP not enough acid
# foo.test: I just love word plays...
PASS: foo.test 4 - Flies fly too :-)
PASS: baz.test 1
END

sed -n '/^PASS: foo\.test/,/^PASS: baz\.test/p' stdout > got

cat exp
cat got
diff exp got

# Sanity check the distribution.
cat > bar.test <<'END'
#!/bin/sh
echo 1..1
echo ok 1
END
echo AM_TEST_LOG_DRIVER_FLAGS = --ignore-exit >> Makefile.in
./config.status Makefile
$MAKE distcheck

rm -f Makefile.in # To avoid a maintainer-check failure.

:
