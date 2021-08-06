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

# TAP support: whitespace normalization (or lack thereof) in the testsuite
# progress output on console.  We keep all of these checks here in a single
# script so that a potential cosmetic change in the output format won't
# force us to tweak dozens of other tests (hopefully).
# See also related test 'tap-todo-skip-whitespace.sh'.

. test-init.sh

cat > Makefile.am << 'END'
TEST_LOG_COMPILER = cat
TESTS =
END

: > exp

spaces_a=${sp}${tab}${tab}${sp}${sp}${tab}
spaces_b=${tab}${tab}${sp}${tab}${sp}${sp}${sp}

#-----------------------------------------------------------------------

echo TESTS += numbers.test >> Makefile.am

cat > numbers.test <<END
1..6
ok${spaces_a}1
ok 2${spaces_b}
ok${spaces_a}3${spaces_b}
not ok${spaces_b}4
not ok 5${spaces_a}
not ok${spaces_b}6${spaces_a}
END

cat >> exp <<END
PASS: numbers.test 1
PASS: numbers.test 2
PASS: numbers.test 3
FAIL: numbers.test 4
FAIL: numbers.test 5
FAIL: numbers.test 6
END

#-----------------------------------------------------------------------

echo TESTS += description.test >> Makefile.am

cat > description.test <<END
1..8
ok${spaces_a}+foo
ok +bar${spaces_b}
ok${spaces_a}+baz${spaces_b}
not ok${spaces_b}-foo
not ok -bar${spaces_a}
not ok${spaces_b}-baz${spaces_a}
ok  u${spaces_b}v${spaces_a}w${sp}
not ok${spaces_a}x${spaces_a}y${tab}z${tab}
END

cat >> exp <<END
PASS: description.test 1 +foo
PASS: description.test 2 +bar
PASS: description.test 3 +baz
FAIL: description.test 4 -foo
FAIL: description.test 5 -bar
FAIL: description.test 6 -baz
PASS: description.test 7 u${spaces_b}v${spaces_a}w
FAIL: description.test 8 x${spaces_a}y${tab}z
END

#-----------------------------------------------------------------------

# "Bail out!" magic.

echo TESTS += bailout.test >> Makefile.am

cat > bailout.test <<END
1..1
Bail out!${tab}${sp}${sp}${tab}We're out of disk space.
ok 1
END

cat >> exp <<END
ERROR: bailout.test - Bail out! We're out of disk space.
END

echo TESTS += bailout2.test >> Makefile.am

cat > bailout2.test <<END
1..1
Bail out!foo${tab}${sp}
ok 1
END

cat >> exp <<END
ERROR: bailout2.test - Bail out! foo
END

#-----------------------------------------------------------------------

# Diagnostic lines.

echo AM_TEST_LOG_DRIVER_FLAGS = --comments >> Makefile.am
echo TESTS += cmnt.test >> Makefile.am

cat > cmnt.test <<END
1..1
ok 1
#Leading whitespace gets added
#   ${tab}${tab} ${tab}Extra leading whitespace is stripped
# Trailing whitespace is stripped ${tab}   ${tab}${tab}
# Middle${tab}whitespace  is${tab}  ${tab}${tab}    kept
# ${tab} And  only${tab}middle ${tab}whitespace  ${tab}${tab} ${tab}
END

cat >> exp <<END
PASS: cmnt.test 1
# cmnt.test: Leading whitespace gets added
# cmnt.test: Extra leading whitespace is stripped
# cmnt.test: Trailing whitespace is stripped
# cmnt.test: Middle${tab}whitespace  is${tab}  ${tab}${tab}    kept
# cmnt.test: And  only${tab}middle ${tab}whitespace
END

#-----------------------------------------------------------------------

# TODO: we should have more checks here ... (but let's not over-do FTM).

#-----------------------------------------------------------------------

chmod a+x *.test

. tap-setup.sh

# We don't care about exit status or number of test results, they
# should be checked for in many other tests.
run_make -O -e FAIL check

LC_ALL=C sort exp > t
mv -f t exp

# We need the sort below to account for parallel make usage.
LC_ALL=C grep '[a-z0-9][a-z0-9]*\.test' stdout | LC_ALL=C sort > got

cat exp
cat got
diff exp got

:
