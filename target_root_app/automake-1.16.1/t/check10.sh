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

# Check singular and plural in test summaries.

# This test only makes sense for the older serial testsuite driver.
am_serial_tests=yes
. test-init.sh

cat >> configure.ac << 'END'
AC_OUTPUT
END

cat > Makefile.am << 'END'
TESTS = fail pass skip xfail xpass fail2 pass2 skip2 xfail2 xpass2
XFAIL_TESTS = xfail xpass xfail2 xpass2
END

cat > pass <<'END'
#! /bin/sh
exit 0
END
cat > fail <<'END'
#! /bin/sh
exit 1
END
cat > skip <<'END'
#! /bin/sh
exit 77
END
chmod a+x pass fail skip
cp pass pass2
cp pass xpass
cp xpass xpass2
cp fail xfail
cp fail fail2
cp xfail xfail2
cp skip skip2

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure
(
  run_make -e IGNORE TESTS=pass check
  run_make -e IGNORE TESTS=fail check
  run_make -e IGNORE TESTS=skip check
  run_make -e IGNORE TESTS=xfail check
  run_make -e IGNORE TESTS=xpass check
  run_make -e IGNORE TESTS="pass pass2" check
  run_make -e IGNORE TESTS="fail fail2" check
  run_make -e IGNORE TESTS="skip skip2" check
  run_make -e IGNORE TESTS="xfail xfail2" check
  run_make -e IGNORE TESTS="xpass xpass2" check
  run_make -e IGNORE TESTS='pass skip xfail' check
  run_make -e IGNORE check
) >stdout || { cat stdout; exit 1; }
cat stdout

grep '1 [tT]ests' stdout && exit 1
grep '[02-9] [tT]est ' stdout && exit 1
grep '1 .* were ' stdout && exit 1
grep '[02-9].* was .*run' stdout && exit 1
grep 'All 1 ' stdout && exit 1
$EGREP '1 (un)?expected (failures|passes)' stdout && exit 1
$EGREP '[^1] (un)?expected (failure|pass)\)' stdout && exit 1

:
