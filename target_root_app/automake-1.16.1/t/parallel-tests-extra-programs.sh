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

# Parallel test harness: check that $(TESTS) can lazily depend on
# (or even be) $(EXTRA_PROGRAMS).

required='cc native'
. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AC_OUTPUT
END

# Will be extended later.
cat > Makefile.am << 'END'
TEST_EXTENSIONS = .bin .test
EXTRA_PROGRAMS =
TESTS =
END

#
# Now try various kinds of test dependencies ...
#

# 1. A program that is also a test, and whose source files
#    already exist.

cat >> Makefile.am <<'END'
EXTRA_PROGRAMS += foo.bin
TESTS += foo.bin
foo_bin_SOURCES = foo.c
END

cat > foo.c <<'END'
#include <stdio.h>
int main (void)
{
  printf ("foofoofoo\n");
  return 0;
}
END

# 2. A program that is also a test, and whose source files
#    are buildable by make.
cat >> Makefile.am <<'END'
EXTRA_PROGRAMS += bar.bin
TESTS += bar.bin
bar_bin_SOURCES = bar.c
bar.c: foo.c
	sed -e 's/foofoofoo/barbarbar/' foo.c > $@
END

# 3. A test script that already exists, whose execution depends
#    on a program whose source files already exist and which is
#    not itself a test.
cat >> Makefile.am <<'END'
EXTRA_PROGRAMS += y
TESTS += baz.test
baz.log: y$(EXEEXT)
END

cat > baz.test <<'END'
#!/bin/sh
$srcdir/y "$@" | sed 's/.*/&ep&ep&ep/'
END
chmod a+x baz.test

cat > y.c <<'END'
#include <stdio.h>
int main (void)
{
  printf ("y\n");
  return 0;
}
END

# 4. A program that is also a test, but whose source files
#    do not exit and are not buildable by make.

cat >> Makefile.am <<'END'
EXTRA_PROGRAMS += none.bin
TESTS += none.bin
none_bin_SOURCES = none.c
END

#
# Setup done, go with the tests.
#

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure

# What we check now:
#  1. even if we cannot build the 'none.bin' program, all the other
#     test programs should be built, and all the other tests should
#     be run;
#  2. still, since we cannot create the 'none.log' file, the
#    'test-suite.log' file shouldn't be created (as it depends
#     on *all* the test logs).

run_make -E -O -e IGNORE -- -k check
ls -l
if using_gmake; then
  test $am_make_rc -gt 0 || exit 1
else
  # Don't trust exit status of "make -k" for non-GNU make.
  $MAKE check && exit 1
  : For shells with busted 'set -e'.
fi

# Files that should have been created, with the expected content.
cat bar.c
grep foofoofoo foo.log
grep barbarbar bar.log
grep yepyepyep baz.log
# Files that shouldn't have been created.
test ! -e none.log
test ! -e test-suite.log
# Expected testsuite progress output.
grep '^PASS: baz\.test$' stdout
# Don't anchor the end of the next two patterns, to allow for non-empty
# $(EXEEXT).
grep '^PASS: foo\.bin' stdout
grep '^PASS: bar\.bin' stdout
# Expected error messages from make.  Some make implementations (e.g.,
# FreeBSD make) seem to print the error on stdout instead, so check for
# it there as well.
$EGREP 'none\.(bin|o|c)' stderr stdout

# What we check now:
#  1. if we make the last EXTRA_PROGRAM buildable, the failed tests
#     pass;
#  2. on a lazy re-run, the passed tests are not re-run, and
#  3. their log files are not updated or touched.

: > stamp
$sleep

echo 'int main (void) { return 0; }' > none.c

run_make -O -e IGNORE check RECHECK_LOGS=
ls -l # For debugging.
test $am_make_rc -eq 0 || exit 1

# For debugging.
stat stamp foo.log bar.log baz.log || :

# Files that shouldn't have been updated or otherwise touched.
is_newest stamp foo.log bar.log baz.log
# Files that should have been created now.
test -f none.log
test -f test-suite.log
# Tests that shouldn't have been re-run.
$EGREP '(foo|bar)\.bin|baz\.test$' stdout && exit 1
# Tests that should have been run.  Again, we don't anchor the end
# of the next pattern, to allow for non-empty $(EXEEXT).
grep '^PASS: none\.bin' stdout

:
