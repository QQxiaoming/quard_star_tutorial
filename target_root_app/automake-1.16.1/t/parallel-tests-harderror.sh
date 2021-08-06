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

# Check parallel-tests features: DISABLE_HARD_ERRORS

. test-init.sh

cat >> configure.ac << 'END'
AC_CONFIG_FILES([sub/Makefile])
AC_OUTPUT
END

mkdir sub

cat > Makefile.am << 'END'
SUBDIRS = . sub
TESTS = foo.test
XFAIL_TESTS = foo.test
EXTRA_DIST = $(TESTS)
END

cat > sub/Makefile.am << 'END'
TESTS = bar.test
XFAIL_TESTS = $(TESTS)
EXTRA_DIST = $(TESTS)
END

cat > foo.test <<'END'
#! /bin/sh
exit 99
END
chmod a+x foo.test
cp foo.test sub/bar.test

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure

# DISABLE_HARD_ERRORS is not defined as a make variable, so that it
# should be possible to define either from the environment or from
# command-line, also when recursive make invocations are involved,
# and also without using the '-e' make flag; this should work also
# for non-GNU make. Moreover, it shouldn't be necessary to define
# DISABLE_HARD_ERRORS to "yes" to really disable hard errors: any
# non-empty value should do.
$MAKE check DISABLE_HARD_ERRORS=yes
DISABLE_HARD_ERRORS=x $MAKE check

# But an empty values for DISABLE_HARD_ERRORS means that hard errors
# are not to be counted like normal failures.

$MAKE check DISABLE_HARD_ERRORS= && exit 1
cat test-suite.log
grep '^ERROR: foo$' test-suite.log

cd sub
$MAKE check DISABLE_HARD_ERRORS= && exit 1
cat test-suite.log
grep '^ERROR: bar$' test-suite.log
cd ..

# Check the distributions.
$MAKE DISABLE_HARD_ERRORS=y distcheck

# Finally, DISABLE_HARD_ERRORS should work also when the developer
# sets it directly in Makefile.am or Makefile.  And its effects
# should remain local to that specific Makefile, obviously.

echo 'DISABLE_HARD_ERRORS = yes' >> Makefile.am
$AUTOMAKE Makefile
./config.status Makefile
VERBOSE=yes $MAKE check && exit 1
grep '^FAIL' test-suite.log && exit 1
grep '^ERROR: bar$' sub/test-suite.log

echo 'DISABLE_HARD_ERRORS = zardoz' >> sub/Makefile
VERBOSE=yes $MAKE check

:
