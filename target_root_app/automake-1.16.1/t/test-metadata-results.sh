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

# Parallel testsuite harness: check APIs for the registering of test
# results in '*.trs' files, as documented in the automake manual.

. test-init.sh

cat >> configure.ac << 'END'
AC_OUTPUT
END

cat > Makefile.am << 'END'
TEST_LOG_DRIVER = ./dummy-driver
TESTS = foo.test bar.test
END

cat > dummy-driver <<'END'
#! /bin/sh
set -e; set -u
while test $# -gt 0; do
  case $1 in
    --log-file) log_file=$2; shift;;
    --trs-file) trs_file=$2; shift;;
    --test-name) test_name=$2; shift;;
    --expect-failure|--color-tests|--enable-hard-errors) shift;;
    --) shift; break;;
     *) echo "$0: invalid option/argument: '$1'" >&2; exit 2;;
  esac
  shift
done
: > $log_file
cp $1 $trs_file
END
chmod a+x dummy-driver

mk_check ()
{
  stat=0
  case $1 in
    -e) stat=$2; shift 2;;
  esac
  run_make -O -e $stat -- check
  # Our dummy driver make no testsuite progress report.
  grep ': .*\.test' stdout && exit 1
  # Nor it writes to the log files.
  test -s foo.log && exit 1
  test -s bar.log && exit 1
  : For 'set -e'.
}

# This must be different from the one defined in 'test/defs', as that
# assumes that the driver does proper testsuite progress reporting.
count_test_results ()
{
  total=ERR pass=ERR fail=ERR xpass=ERR xfail=ERR skip=ERR error=ERR
  eval "$@"
  st=0
  grep "^# TOTAL:  *$total$" stdout || rc=1
  grep "^# PASS:  *$pass$"   stdout || rc=1
  grep "^# XFAIL:  *$xfail$" stdout || rc=1
  grep "^# SKIP:  *$skip$"   stdout || rc=1
  grep "^# FAIL:  *$fail$"   stdout || rc=1
  grep "^# XPASS:  *$xpass$" stdout || rc=1
  grep "^# ERROR:  *$error$" stdout || rc=1
  test $st -eq 0 || exit 1
}

$ACLOCAL
$AUTOCONF
$AUTOMAKE

./configure

# Basic checks.  Also that that ':global-test-result:' fields and
# "old-style" directives with format "RESULT: test-name" are now ignored.

: > foo.test
echo blah blah blah > bar.test
mk_check
count_test_results total=0 pass=0 fail=0 xpass=0 xfail=0 skip=0 error=0

cat > foo.test <<END
:test-global-result: PASS
:test-result: FAIL
END
cat > bar.test <<END
:test-result: SKIP
:test-global-result: ERROR
END
mk_check -e FAIL
count_test_results total=2 pass=0 fail=1 xpass=0 xfail=0 skip=1 error=0

cat > foo.test <<END
FAIL: foo.test
:test-result: PASS
:test-global-result: XPASS
END
echo ERROR: bar.test > bar.test
mk_check
count_test_results total=1 pass=1 fail=0 xpass=0 xfail=0 skip=0 error=0

cat > foo.test <<END
:test-global-result: SKIP
:test-result: FAIL
END
cat > bar.test <<END
:test-global-result: PASS
END
mk_check -e FAIL
count_test_results total=1 pass=0 fail=1 xpass=0 xfail=0 skip=0 error=0

cat > foo.test <<END
:test-result: XFAIL
:test-result: PASS
:test-result: SKIP
END
cat > bar.test <<END
:test-result: SKIP
:test-result: PASS
:test-result: SKIP
:test-result: PASS
:test-result: PASS
END
mk_check
count_test_results total=8 pass=4 fail=0 xpass=0 xfail=1 skip=3 error=0

# Check that all results expected to be supported are *really* supported.

cat > foo.test <<END
:test-result: PASS
:test-result: SKIP
:test-result: XFAIL
:test-result: FAIL
:test-result: XPASS
:test-result: ERROR
END
: > bar.test
mk_check -e FAIL
count_test_results total=6 pass=1 fail=1 xpass=1 xfail=1 skip=1 error=1

cp foo.test bar.test
mk_check -e FAIL
count_test_results total=12 pass=2 fail=2 xpass=2 xfail=2 skip=2 error=2

# Check that we are liberal w.r.t. whitespace use.

: > foo.test
: > bar.test
for RESULT in PASS FAIL XPASS XFAIL SKIP ERROR; do
  sed -e 's/^ *//' -e 's/|//g' >> foo.test <<END
    |:test-result:$RESULT|
    |:test-result: $tab  $RESULT|
    |:test-result:$RESULT  $tab|
    |:test-result:$tab$tab  $RESULT$tab  $tab |
END
  echo "  $tab $tab$tab   :test-result: $RESULT" >> bar.test
done
cat foo.test # For debugging.
cat bar.test # Likewise.
mk_check -e FAIL
count_test_results total=30 pass=5 fail=5 xpass=5 xfail=5 skip=5 error=5

:
