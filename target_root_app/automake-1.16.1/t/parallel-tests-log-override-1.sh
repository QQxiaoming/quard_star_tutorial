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

# Check parallel-tests features: runtime redefinition of $(TEST_SUITE_LOG).

. test-init.sh

cat >> configure.ac <<'END'
AC_OUTPUT
END

cat > Makefile.am <<'END'
TESTS = pass.test skip.test xfail.test
XFAIL_TESTS = xfail.test
EXTRA_DIST = $(TESTS)
END

cat > pass.test <<'END'
#! /bin/sh
exit 0
END

cat > skip.test <<'END'
#! /bin/sh
echo "% test skipped %"
exit 77
END

cat > xfail.test <<'END'
#! /bin/sh
echo "# expected failure #"
exit 1
END

chmod a+x *.test

test_log_edit ()
{
  sed -e "s|^  *$me 1\.0:.*$|  $me 1.0: ???|" \
      -e "s|^=====*|=======================|" $*
}

test_log_expected ()
{
  test_log_edit orig > exp
  test_log_edit $1   > got
  diff exp got || exit 1
  rm -f exp got
}

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure

$MAKE check
ls -l
cat test-suite.log
cp test-suite.log orig

$MAKE clean
test -f test-suite.log && exit 99 # Sanity check.

# Check that we can override the testsuite log file at runtime.
run_make TEST_SUITE_LOG=zardoz.log check
ls -l
test ! -e test-suite.log
cat zardoz.log
test_log_expected zardoz.log
# Sanity check the distribution too (this also does minimal checks on
# VPATH support).
run_make TEST_SUITE_LOG=zardoz.log distcheck

# Check that cleanup rules remove the correct file even when
# user overrides are in place.
cp orig test-suite.log
run_make TEST_SUITE_LOG=zardoz.log clean
ls -l
test ! -e zardoz.log
diff orig test-suite.log

# Check that the default testsuite log doesn't get unduly modified.
# Also check that the testsuite log file doesn't need to be named
# accordingly to the '*.log' pattern.
chmod a-w test-suite.log
run_make TEST_SUITE_LOG=TheLogFile check
ls -l
diff orig test-suite.log
test_log_expected TheLogFile
run_make TEST_SUITE_LOG=TheLogFile clean
ls -l
test ! -e TheLogFile
diff orig test-suite.log

:
