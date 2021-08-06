#! /bin/sh
# Copyright (C) 2003-2018 Free Software Foundation, Inc.
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

# Check that the DejaGnu rules work for a simple program and test case.
# Also check PR 488: Failure of the first of several tools tested.
# From the original bug report:
#   If you use dejagnu for testing and have multiple tools (i.e.,
#   multiple entries in the DEJATOOL variable) then the success/failure
#   of "make check" is only dependent on the success/failure of the
#   tests on the final tool. Thus there may be failures in the tests on
#   previous tools, but at first glance "make check" has passed.

required=runtest
. test-init.sh

cat > hammer << 'END'
#! /bin/sh
echo "Everything looks like a nail to me!"
END
chmod +x hammer

cat > spanner << 'END'
#! /bin/sh
echo "I'm a right spanner!"
END
chmod +x spanner

cat >> configure.ac << 'END'
AC_OUTPUT
END

cat > Makefile.am << 'END'
AUTOMAKE_OPTIONS = dejagnu

DEJATOOL = hammer spanner

AM_RUNTESTFLAGS = HAMMER=$(srcdir)/hammer SPANNER=$(srcdir)/spanner

EXTRA_DIST  = hammer  hammer.test/hammer.exp
EXTRA_DIST += spanner spanner.test/spanner.exp
END

mkdir hammer.test spanner.test

cat > hammer.test/hammer.exp << 'END'
set test test
spawn $HAMMER
expect {
    "Everything looks like a nail to me!" { pass "$test" }
    default { fail "$test" }
}
END

cat > spanner.test/spanner.exp << 'END'
set test test
spawn $SPANNER
expect {
    "I'm a right spanner!" { pass "$test" }
    default { fail "$test" }
}
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE --add-missing

./configure

$MAKE check
test -f hammer.log
test -f hammer.sum
test -f spanner.log
test -f spanner.sum

$MAKE distcheck

# Test for PR 488.
sed 's/E\(verything\)/Not e\1/' hammer > thammer
mv -f thammer hammer
chmod +x hammer

rm -f hammer.log hammer.sum spanner.log spanner.sum
$MAKE check && exit 1
test -f hammer.log
test -f hammer.sum
test -f spanner.log
test -f spanner.sum

:
