#! /bin/sh
# Copyright (C) 1996-2018 Free Software Foundation, Inc.
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

# Make sure ":" works with files automake generates.
# This test is for multiple ":"s.
# See also sister test 'colon2.sh'.

. test-init.sh

cat > configure.ac <<END
AC_INIT([$me], [1.0])
AM_INIT_AUTOMAKE
AC_CONFIG_FILES([Makefile:zardoz.in:two.in:three.in])
AC_OUTPUT
END

cat > zardoz.am <<END
.PHONY: dummy
dummy:
END

: > two.in
: > three.in

$ACLOCAL
$AUTOMAKE

# Automake should have created zardoz.in.
test -f zardoz.in

# The generated file should refer to zardoz.in and zardoz.am, but
# never just "zardoz".
$FGREP 'zardoz.am' zardoz.in
$FGREP 'zardoz.in' zardoz.in
sed -e 's|zardoz\.am|zrdz.am|g' \
    -e 's|zardoz\.in|zrdz.in|g' \
  <zardoz.in | $FGREP 'zardoz' && exit 1

# The configure-generated Makefile should depend on zardoz.in, two.in and
# three.in.  The automake-generated zardoz.in should depend on zardoz.am.
# Let's do this check the right way by running configure and make.

str='%% ZaR DoZ %%'
str2='// 2_TwO_2 //'
str3='== @thr33@ =='

$AUTOCONF
./configure
test -f Makefile # Sanity check.

$MAKE dummy
# Again, make sure that the generated Makefile do not unduly
# refer to 'zardoz' .
$MAKE -n zardoz && exit 1

$sleep
cat >> zardoz.am <<END
check-local: testam testin testmk test2 test3
.PHONY: testam testin test2 testmk test3
testam:
	grep '$str' \$(srcdir)/zardoz.am
testin:
	grep '$str' \$(srcdir)/zardoz.in
testmk:
	grep '$str' Makefile
test2:
	grep '$str2' \$(srcdir)/two.in
test3:
	grep '$str3' \$(srcdir)/three.in
END
echo "# $str2" >> two.in
echo "# $str3" >> three.in
$MAKE Makefile # For non-GNU make.
$MAKE testam testin testmk test2 test3

$MAKE distcheck

:
