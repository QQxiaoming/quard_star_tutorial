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

# Test the "make recheck" semantics for custom test drivers, as documented
# in the Automake manual.

. test-init.sh

cat >> configure.ac << 'END'
AC_OUTPUT
END

cat > Makefile.am << 'END'
LOG_DRIVER = ./dummy-driver
TEST_EXTENSIONS =
TESTS =
END

#
# Tests to be re-run by "make recheck"
#

: > y-1
echo "foo bar" > y-2
echo ":recheck:" > y-3
echo ":recheck:yes" > y-4
echo ":recheck: who cares" > y-5
echo ":recheck: $tab   y" > y-6
echo ":recheck: yeah!$tab$tab " > y-7
cat > y-10 <<END
:foo:
:recheck: ???
END
cat > y-11 <<END
:recheck: YES
:foo:
END
cat > y-12 <<END
foo
:recheck:yes

bar
zardoz
END

echo "  $tab $tab$tab    :recheck: yes" > y-8

# The :test-result: fields and the fist line of the log should be
# irrelevant for the decision of whether "make recheck" should or
# should not re-run a test.

echo ":test-result: PASS" > y-100

echo "PASS: y-101"

cat > y-102 <<END
PASS: y-102
===========

:test-result: PASS
END

#
# Tests *not* to be re-run by "make recheck"
#

echo ":recheck:no" > n-1
echo ":recheck: no " > n-2
echo ":recheck: $tab   no" > n-3
echo ":recheck: no $tab$tab " > n-4
cat > n-5 <<END
:foo:
:recheck:no
END
cat > n-6 <<END
:recheck: no
:foo:
END
cat > n-7 <<END
foo
:recheck: no$tab$tab

bar
zardoz
END

echo "  $tab $tab$tab    :recheck: no" > n-8

# The :test-result: fields should be irrelevant for the decision of
# whether "make recheck" should or should not re-run a test.
cat > n-100 <<END
:test-result: FAIL
:test-result: XPASS
:test-result: ERROR
:test-result: UNKNOWN
:recheck: no
END

rechecked=$(echo y-[0-9]*)

for t in [yn]-[0-9]*; do echo $t; done \
  | sed 's/.*/TESTS += &/' >> Makefile.am

cat Makefile.am # For debugging.

cat > dummy-driver <<'END'
#!/bin/sh
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
: > $test_name.run
: > $log_file
cp $1 $trs_file
END
chmod a+x dummy-driver

$ACLOCAL
$AUTOCONF
$AUTOMAKE

./configure

# The ':test-result:' fields should be ignored by "make recheck",
# but should cause the testsuite report to detect errors.
$MAKE check && exit 1
ls -l
for t in $tests; do test -f $t.run; done
rm -f *.run

# But now the tests that actually get re-run have only ':test-result:'
# fields indicating success, so "make recheck" must pass.  Still, the
# next "make recheck" call should still re-run the same set of tests.
for iteration in 1 2; do
  using_gmake || $sleep # Required by BSD make.
  $MAKE recheck
  ls -l
  for t in $rechecked; do test -f $t.run; done
  find . -name 'n-*.run' | grep . && exit 1
  : For shells with busted 'set -e'.
done

:
