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
# See also sister test 'colon3.sh'.

. test-init.sh

cat > configure.ac <<END
AC_INIT([$me], [1.0])
AM_INIT_AUTOMAKE
AC_CONFIG_FILES([Makefile:zardoz.in])
AC_OUTPUT
END

cat > zardoz.am <<END
.PHONY: dummy
dummy:
END

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

$AUTOCONF
./configure
test -f Makefile # Sanity check.

$MAKE dummy
# Again, make sure that the generated Makefile do not unduly
# refer to 'zardoz' .
$MAKE -n zardoz && exit 1

$sleep
cat >> zardoz.am <<END
check-local: testam testin testmk
.PHONY: testam testin testmk
testam:
	grep '$str' \$(srcdir)/zardoz.am
testin:
	grep '$str' \$(srcdir)/zardoz.in
testmk:
	grep '$str' Makefile
END
$MAKE Makefile # For non-GNU make.
$MAKE testam testin testmk

$MAKE distcheck

:
