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

# Custom test drivers: "abstract" XFAIL_TESTS support.

. test-init.sh

cat >> configure.ac <<'END'
AC_SUBST([nihil], [])
AC_SUBST([ac_xfail_tests], ['x5.test x6$(test_suffix)'])
AC_CONFIG_FILES([sub1/Makefile sub2/Makefile])
AC_OUTPUT
END

cat > Makefile.am <<'END'
SUBDIRS = . sub1 sub2
TEST_LOG_DRIVER = $(srcdir)/td
TESTS = pass.test xfail.test
XFAIL_TESTS = xfail.test
END

mkdir sub1 sub2

cat > sub1/Makefile.am <<END
empty =

TEST_LOG_DRIVER = \$(top_srcdir)/td

# XFAIL_TESTS should gracefully handle TAB characters, and multiple
# whitespaces.
XFAIL_TESTS =\$(empty)${tab}x1.test x2.test${tab}x3.test${tab}\
x4.test ${tab} x5.test              x6.test${tab}\$(empty)

TESTS = pass.test x1.test x2.test x3.test x4.test x5.test x6.test
END

cat > sub2/Makefile.am <<'END'
AUTOMAKE_OPTIONS = -Wno-portability-recursive

TEST_LOG_DRIVER = $(srcdir)/../td

# XFAIL_TESTS should gracefully AC_SUBST @substitution@ and
# make variables indirections.
an_xfail_test = x1.test
test_suffix = .test
v0 = x3.test
v1 = v
v2 = 0
XFAIL_TESTS = $(an_xfail_test) x2.test @nihil@ x3${test_suffix}
XFAIL_TESTS += $($(v1)$(v2)) x4.test @ac_xfail_tests@

TESTS = pass.test x1.test x2.test x3.test x4.test x5.test x6.test
END

cat > pass.test <<'END'
#!/bin/sh
exit 0
END

cat > xfail.test <<'END'
#!/bin/sh
exit 1
END

chmod a+x pass.test xfail.test

cp pass.test sub1/pass.test
cp pass.test sub2/pass.test

for i in 1 2 3 4 5 6; do
  cp xfail.test sub1/x$i.test
  cp xfail.test sub2/x$i.test
done

cat > td <<'END'
#! /bin/sh
set -e; set -u
test_name=INVALID
log_file=/dev/null
trs_file=/dev/null
expect_failure=no
while test $# -gt 0; do
  case $1 in
    --test-name) test_name=$2; shift;;
    --expect-failure) expect_failure=$2; shift;;
    --log-file) log_file=$2; shift;;
    --trs-file) trs_file=$2; shift;;
    # Ignored.
    --color-tests) shift;;
    --enable-hard-errors) shift;;
    # Explicitly terminate option list.
    --) shift; break;;
    # Shouldn't happen
    *) echo "$0: invalid option/argument: '$1'" >&2; exit 2;;
  esac
  shift
done
st=0
"$@" || st=$?
case $st,$expect_failure in
  0,no)
    echo "PASS: $test_name" | tee "$log_file"
    echo ":test-result: PASS" > "$trs_file"
    ;;
  1,no)
    echo "FAIL: $test_name" | tee "$log_file"
    echo ":test-result: FAIL" > "$trs_file"
    ;;
  0,yes)
    echo "XPASS: $test_name" | tee "$log_file"
    echo ":test-result: XPASS" > "$trs_file"
    ;;
  1,yes)
    echo "XFAIL: $test_name" | tee "$log_file"
    echo ":test-result: XFAIL" > "$trs_file"
    ;;
  *)
    echo "INTERNAL ERROR" >&2
    exit 99
    ;;
esac
END
chmod a+x td

$ACLOCAL
$AUTOCONF
$AUTOMAKE

./configure

run_make -O check
test $(grep -c '^PASS:'  stdout) -eq 3
test $(grep -c '^XFAIL:' stdout) -eq 13

for dir in sub1 sub2; do
  cd $dir
  cp pass.test x1.test
  cp x2.test pass.test
  run_make -O -e FAIL check
  test "$(cat pass.trs)" = ":test-result: FAIL"
  test "$(cat x1.trs)"   = ":test-result: XPASS"
  test "$(cat x2.trs)"   = ":test-result: XFAIL"
  grep '^FAIL: pass\.test$' stdout
  grep '^XPASS: x1\.test$'  stdout
  grep '^XFAIL: x2\.test$'  stdout
  count_test_results total=7 pass=0 xpass=1 fail=1 xfail=5 skip=0 error=0
  cd ..
done

:
