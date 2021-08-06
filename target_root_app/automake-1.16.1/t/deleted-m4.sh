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

# The stub rules emitted to work around the "deleted header problem"
# for '.m4' files shouldn't prevent the remake rules from correctly
# erroring out when a still-required file is missing.
# See also discussion about automake bug#9768.

. test-init.sh

cat >> configure.ac <<'END'
m4_include([foobar.m4])
AC_OUTPUT
END

: > Makefile.am

echo 'm4_include([zardoz.m4])' > foobar.m4
: > zardoz.m4

$ACLOCAL
$AUTOCONF
$AUTOMAKE

./configure
$MAKE

rm -f zardoz.m4
run_make -e FAIL -M
# This error will come from aclocal, not make, so we can be stricter
# in our grepping of it.
grep ' foobar\.m4:1:.*zardoz\.m4.*does not exist' output
# No spurious errors, please.
$FGREP -v ' foobar.m4:1:' output | $FGREP 'foobar.m4' && exit 1

# Try with one less indirection.
: > foobar.m4
$ACLOCAL --force
$AUTOCONF
./configure
$MAKE # Sanity check.
rm -f foobar.m4
run_make -e FAIL -M
# This error will come from aclocal, not make, so we can be stricter
# in our grepping of it.
grep 'foobar\.m4.*does not exist' output
# No spurious errors, please (ok, this is really paranoid).
$FGREP 'zardoz.m4' output && exit 1

:
