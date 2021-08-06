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

# Custom test drivers and parallel test harness: check the documented
# semantics for deciding when the content of a test log file should be
# copied in the global test-suite.log file.  Currently, this is done
# with the use of the reStructuredText field ':copy-in-global-log:' in
# the associated '.trs' files.

. test-init.sh

cat >> configure.ac << 'END'
AC_OUTPUT
END

cat > Makefile.am << 'END'
TEST_LOG_DRIVER = ./passthrough-driver
TEST_LOG_COMPILER = $(SHELL) -e
END

cat > passthrough-driver <<'END'
#!/bin/sh
set -e; set -u;
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
echo "$test_name: RUN"
"$@" >$log_file 2>&1 5>$trs_file
END
chmod a+x passthrough-driver

# The ':test-result:' and ':recheck:' fields and the first line of the
# log file should be be irrelevant for the decision of whether a test
# output is to be copied in the 'test-suite.log'.

cat > no-1.test <<END
echo :test-result: SKIP >&5
echo :copy-in-global-log: no >&5
echo :test-result: FAIL >&5
echo :test-result: XPASS >&5
echo not seen 1
END

# In the last line, with leading and trailing whitespace in the value.
cat > no-2.test <<END
echo ":test-result: FAIL" >&5
echo "not seen 2"
echo ":recheck: yes" >&5
echo ":copy-in-global-log:$tab $tab no   $tab" >&5
END

for RES in XPASS FAIL XFAIL SKIP ERROR UNKNOWN; do
  unindent > $RES.test <<END
    echo :test-result: $RES >&5
    echo :copy-in-global-log: no >&5
    echo not seen $RES
END
done

# In the first line, with no whitespace.
cat > no-3.test <<END
echo :copy-in-global-log:no >&5
echo ":test-result: FAIL" >&5
echo "not seen 3"
END

# Leading whitespace before the field.
cat > no-4.test <<END
echo ":test-result: FAIL" >&5
echo "  $tab $tab$tab   :copy-in-global-log: no" >&5
echo "not seen 4"
END

cat > yes-1.test <<END
echo :test-result: PASS >&5
echo :copy-in-global-log: yes >&5
echo seen yes 1
END

# A lacking ':copy-in-global-log:' implies that the content of
# the log file should be copied.
cat > yes-2.test <<END
echo :test-result: PASS >&5
echo seen yes 2
END

# Three corner cases.

cat > corn-1.test <<END
echo seen corn 1
echo ':copy-in-global-log:' >&5
END

cat > corn-2.test <<END
echo seen corn 2
echo '$tab $tab$tab' >&5
END

cat > corn-3.test <<'END'
echo seen corn 31
echo ':copy-in-global-log:#@%!' >&5
echo seen corn 32
END

echo TESTS = *.test >> Makefile.am

$ACLOCAL
$AUTOCONF
$AUTOMAKE

./configure

# We don't care about the exit status of "make check" here, that
# should be checked in other tests.
$MAKE check || :
cat test-suite.log
grep '^seen yes 1$' test-suite.log
grep '^seen yes 2$' test-suite.log
grep '^seen corn 1$' test-suite.log
grep '^seen corn 2$' test-suite.log
grep '^seen corn 31$' test-suite.log
grep '^seen corn 32$' test-suite.log
$FGREP 'not seen' test-suite.log && exit 1

:
