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

# Check that "make check" fails, when we invoke DejaGnu tests with --status
# (to detect TCL errors) on a file with TCL errors.

required=runtest
. test-init.sh

runtest --help | grep '.*--status' \
  || skip_ "dejagnu lacks support for '--status'"

cat > failtcl << 'END'
#! /bin/sh
echo whatever
END
chmod +x failtcl

cat >> configure.ac << 'END'
AC_OUTPUT
END

cat > Makefile.am << 'END'
AUTOMAKE_OPTIONS = dejagnu
DEJATOOL = failtcl
AM_RUNTESTFLAGS = --status FAILTCL=$(srcdir)/failtcl
END

mkdir failtcl.test
cat > failtcl.test/failtcl.exp << 'END'
set test test
spawn $FAILTCL
expect {
    default { pass "$test" }
# Oops, no closing brace.
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE --add-missing

./configure

$MAKE check && exit 1
test -f failtcl.log
test -f failtcl.sum
$FGREP 'missing close-brace' failtcl.sum

:
