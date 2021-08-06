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

# Check that "make check" fails, when a DejaGnu test fails.

required=runtest
. test-init.sh

cat > faildeja << 'END'
#! /bin/sh
echo whatever
END
chmod +x faildeja

cat >> configure.ac << 'END'
AC_OUTPUT
END

cat > Makefile.am << 'END'
AUTOMAKE_OPTIONS = dejagnu
DEJATOOL = faildeja
AM_RUNTESTFLAGS = FAILDEJA=$(srcdir)/faildeja
END

mkdir faildeja.test
cat > faildeja.test/faildeja.exp << 'END'
set test failing_deja_test
spawn $FAILDEJA
expect {
    default { fail "$test" }
}
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE --add-missing

./configure

$MAKE check && exit 1
test -f faildeja.log
test -f faildeja.sum
$FGREP 'FAIL: failing_deja_test' faildeja.sum

:
