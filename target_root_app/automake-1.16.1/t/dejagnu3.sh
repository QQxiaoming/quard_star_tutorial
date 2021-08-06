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

required=runtest
. test-init.sh

cat > hammer << 'END'
#! /bin/sh
echo "Everything looks like a nail to me!"
END

chmod +x hammer

cat >> configure.ac << 'END'
AC_OUTPUT
END

cat > Makefile.am << 'END'
AUTOMAKE_OPTIONS = dejagnu
DEJATOOL = hammer
AM_RUNTESTFLAGS = HAMMER=$(srcdir)/hammer
EXTRA_DIST = hammer hammer.test/hammer.exp
END

mkdir hammer.test

cat > hammer.test/hammer.exp << 'END'
set test test
spawn $HAMMER
expect {
    "Everything looks like a nail to me!" { pass "$test" }
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

$MAKE distcheck

# Ensure that the envvar RUNTESTFLAGS is used.
# Report from Mark Mitchell.
RUNTESTFLAGS=--unknown-runtest-option; export RUNTESTFLAGS
run_make -M -e FAIL check
$FGREP 'unknown-runtest-option' output

:
