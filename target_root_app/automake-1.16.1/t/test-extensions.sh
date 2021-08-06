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

# Make sure that Automake diagnose invalid entries in TEST_EXTENSIONS,
# and do not diagnose valid (albeit more unusual) ones.
# See automake bug#9400.

. test-init.sh

cat >> configure.ac <<'END'
AC_OUTPUT
END

$ACLOCAL
$AUTOCONF

valid_extensions='sh T t1 _foo BAR x_Y_z _'

echo TESTS = > Makefile.am
echo " $valid_extensions" \
  | sed -e 's/ / ./g' -e 's/^/TEST_EXTENSIONS =/' >> Makefile.am
cat Makefile.am # For debugging.

$AUTOMAKE -a

grep -i 'log' Makefile.in # For debugging.

for lc in $valid_extensions; do
  uc=$(echo $lc | tr abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ)
  $FGREP "\$(${uc}_LOG_COMPILER)" Makefile.in
  grep "^${uc}_LOG_COMPILE =" Makefile.in
  grep "^\.${lc}\.log:" Makefile.in
done

# The produced Makefile is not broken.
./configure
$MAKE all check

cat > Makefile.am << 'END'
TESTS = foo.test bar.sh
TEST_EXTENSIONS  = .test mu .x-y a-b .t.1 .sh .6c .0 .11 .= @suf@ .@ext@
TEST_EXTENSIONS += .= .t33 .a@b _&_
END

AUTOMAKE_fails
for suf in mu .x-y a-b .t.1 .6c .0 .11  @suf@ .@ext@ '.=' '_&_'; do
  suf2=$(printf '%s\n' "$suf" | sed -e 's/\./\\./')
  $EGREP "^Makefile\.am:2:.*invalid test extension.* $suf2( |$)" stderr
done

# Verify that we accept valid suffixes, even if intermixed with
# invalid ones.
$EGREP '\.(sh|test|t33)' stderr && exit 1

# Verify that we don't try to handle invalid suffixes.
$EGREP '(LOG_COMPILER|non-POSIX var|bad character)' stderr && exit 1

:
