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

# TAP result lines whose description is a number.

. test-init.sh

. tap-setup.sh

# Some random numbers to be used as test names.  The definitions below are
# selected so that $x<n> != <n> for every n >= 1.  We can't use positional
# parameters for this unfortunately, since Solaris 10 /bin/sh doesn't allow
# the dereferencing of parameters from ${10} onwards.
x1=5 x2=7 x3=45 x4=11 x5=7 x6=3 x7=6 x8=9 x9=1000 x10=1

cat > all.test <<END
1..10
ok 1 ${x1}
ok - ${x2}
not ok 3 ${x3}
not ok - ${x4}
ok 5 ${x5} # SKIP
ok - ${x6} # SKIP
not ok 7 ${x7} # TODO
not ok - ${x8} # TODO
ok 9 ${x9} # TODO
ok - ${x10} # TODO
END

cat > exp <<END
PASS: all.test 1 ${x1}
PASS: all.test 2 - ${x2}
FAIL: all.test 3 ${x3}
FAIL: all.test 4 - ${x4}
SKIP: all.test 5 ${x5} # SKIP
SKIP: all.test 6 - ${x6} # SKIP
XFAIL: all.test 7 ${x7} # TODO
XFAIL: all.test 8 - ${x8} # TODO
XPASS: all.test 9 ${x9} # TODO
XPASS: all.test 10 - ${x10} # TODO
END

run_make -O -e FAIL check
count_test_results total=10 pass=2 fail=2 xpass=2 xfail=2 skip=2 error=0

$FGREP ': all.test' stdout > got

cat exp
cat got
diff exp got


:
