#! /bin/sh
# Copyright (C) 2008-2018 Free Software Foundation, Inc.
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

# Check subdir TESTS.

# For gen-testsuite-part: ==> try-with-serial-tests <==
required='cc native'
. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AC_OUTPUT
END

cat > Makefile.am << 'END'
AUTOMAKE_OPTIONS = subdir-objects
TESTS = foo sub/foo $(check_PROGRAMS)
XFAIL_TESTS = foo sub/baz
check_PROGRAMS = bar sub/bar baz sub/baz
END

mkdir sub

cat > foo <<'END'
#! /bin/sh
test -f "$srcdir/Makefile.am"
END
cat > sub/foo <<'END'
#! /bin/sh
test -f "$srcdir/Makefile.am"
END
chmod a+x foo sub/foo

cat > bar.c <<'END'
int main (void) { return 0; }
END
cat > sub/bar.c <<'END'
int main (void) { return 0; }
END
cat > baz.c <<'END'
#include <stdlib.h>
int main (void) { return EXIT_FAILURE; }
END
cat > sub/baz.c <<'END'
#include <stdlib.h>
int main (void) { return EXIT_FAILURE; }
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure
run_make -E -O -e FAIL check
grep 'XPASS.* foo$' stdout
grep '^[^X]*PASS.* sub/foo$' stdout
grep '^[^X]*PASS.* bar' stdout
grep '^[^X]*PASS.* sub/bar' stdout
grep '^[^X]*FAIL.* baz' stdout
grep 'XFAIL.* sub/baz' stdout
# The parallel test driver should cause circular dependencies.
# Look for known warnings from a couple of 'make' implementations.
grep -i 'circular.*dependency' stderr && exit 1
grep -i 'graph cycles' stderr && exit 1

$MAKE distclean

mkdir build
cd build
../configure
run_make -O -e FAIL check
# Note: we are not grepping for the space in the lines from the 'foo'
# tests, due to the Solaris make VPATH rewriting (if we fix that, we
# can still write a separate test for it).
grep 'XPASS.*foo$' stdout
grep '^[^X]*PASS.*sub/foo$' stdout
grep '^[^X]*PASS.* bar' stdout
grep '^[^X]*PASS.* sub/bar' stdout
grep '^[^X]*FAIL.* baz' stdout
grep 'XFAIL.* sub/baz' stdout

:
