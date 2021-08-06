#! /bin/sh
# Copyright (C) 2002-2018 Free Software Foundation, Inc.
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

# Test Automake style tests.

# For gen-testsuite-part: ==> try-with-serial-tests <==
. test-init.sh

cat >> configure.ac << 'END'
AC_CONFIG_FILES([dir/Makefile])
AC_OUTPUT
END

mkdir dir

cat > Makefile.am << 'END'
SUBDIRS = dir
TESTS = \
  subrun.sh
subrun.sh:
	(echo '#! /bin/sh'; echo 'dir/echo.sh') > $@
	chmod +x $@
CLEANFILES = subrun.sh
END

cat > dir/Makefile.am << 'END'
check_SCRIPTS = echo.sh
echo.sh:
	(echo '#! /bin/sh'; echo 'echo Hello') > $@
	chmod +x $@
CLEANFILES = echo.sh
END

test x"$am_serial_tests" = x"yes" || cp "$am_scriptdir/test-driver" .

$ACLOCAL
$AUTOCONF
$AUTOMAKE
./configure

run_make -O check
grep '^PASS: subrun\.sh *$' stdout
grep 'PASS.*echo\.sh' stdout && exit 1

# 'check' should depend directly on 'check-am' (similar tests are
# in 'check.sh' and 'built-sources-check.sh').
$EGREP '^check:.* check-recursive( |$)' Makefile.in
$EGREP '^check:.* check-am( |$)' dir/Makefile.in

# Make sure subrun.sh is still on its line as above.  This means Automake
# hasn't rewritten the TESTS line unnecessarily (we can tell, because all
# Automake variables are reformatted by VAR_PRETTY).
grep '^  subrun\.sh$' Makefile.in

:
