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

# Parallel testsuite harness: check APIs for the registering the
# "global test result" in '*.trs' files, as documented in the automake
# manual.

. test-init.sh

cat >> configure.ac << 'END'
AC_OUTPUT
END

cat > Makefile.am << 'END'
TEST_EXTENSIONS = .test .x
TEST_LOG_DRIVER = ./dummy-driver
X_LOG_DRIVER = ./dummy-driver
TESTS = foo.test zar-doz.test
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
echo logloglog > $log_file
cp $1 $trs_file
END
chmod a+x dummy-driver

# Do this in a subroutine to avoid quoting problem in the backticked
# command substitution below.
get_escaped_line()
{
   sed -e 's,[$^/\\\.],\\&,g' -e 1q "$@"
}

have_result ()
{
   cat > exp; echo >> exp; echo logloglog >> exp
   eline=$(get_escaped_line exp)
   sed -n -e "/^$eline$/,/^logloglog$/p" test-suite.log > got
   cat exp; cat got
   diff exp got
}

$ACLOCAL
$AUTOCONF
$AUTOMAKE

./configure

: Basic checks.

echo :global-test-result: PASS > foo.test
echo :global-test-result: ERROR > zar-doz.x

$MAKE check
cat test-suite.log

have_result <<END
PASS: foo
=========
END

have_result <<END
ERROR: zar-doz
==============
END

: Try usage documented in the manual.

echo :global-test-result: PASS/SKIP > foo.test
echo :global-test-result: ALMOST PASSED > zar-doz.x

$MAKE check
cat test-suite.log

have_result <<END
PASS/SKIP: foo
==============
END

have_result <<END
ALMOST PASSED: zar-doz
======================
END

: Fields ':test-result:' does not interfere with the global test result.

cat > foo.test << 'END'
:test-result: FAIL
:global-test-result: PASS
:test-result: ERROR
END

cat > zar-doz.x << 'END'
:global-test-result: FAIL
:test-result: SKIP
:test-result: XFAIL
END

$MAKE check && exit 1
cat test-suite.log

have_result <<END
PASS: foo
=========
END

have_result <<END
FAIL: zar-doz
=============
END

: What happens when ':global-test-result:' is absent.

cat > foo.test << 'END'
:test-result: PASS
:test-result: ERROR
END
: > zar-doz.x

$MAKE check && exit 1
cat test-suite.log

have_result <<END
RUN: foo
========
END

have_result <<END
RUN: zar-doz
============
END

# Leading and trailing whitespace gets eaten/normalized.

echo ":global-test-result:SKIP${tab}   ${tab}${tab}" > foo.test
echo ":global-test-result:${tab}   ${tab}XFAIL  ${tab}   " > zar-doz.x

$MAKE check
cat test-suite.log

have_result <<END
SKIP: foo
=========
END

have_result <<END
XFAIL: zar-doz
==============
END

# Whitespaces before and after ':global-test-result:' are handled OK.

echo "   $tab:global-test-result:PASS" > foo.test
echo "${tab}${tab}:global-test-result:${tab}   ${tab}SKIP" > zar-doz.x

$MAKE check
cat test-suite.log

have_result <<END
PASS: foo
=========
END

have_result <<END
SKIP: zar-doz
=============
END

:
