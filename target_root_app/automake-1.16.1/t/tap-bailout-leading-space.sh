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

# A "Bail out!" directive that is preceded by whitespace should still
# be recognized.

. test-init.sh
. tap-setup.sh

cat > a.test <<END
1..1
ok 1
 Bail out!
END

cat > b.test <<END
1..1
ok 1 # SKIP
${tab}Bail out!
END

cat > c.test <<END
1..1
  ${tab}  ${tab}${tab}Bail out!   FUBAR! $tab
END

cat >> exp <<END
PASS: a.test 1
ERROR: a.test - Bail out!
SKIP: b.test 1
ERROR: b.test - Bail out!
ERROR: c.test - Bail out! FUBAR!
END

run_make -O -e FAIL TESTS='a.test b.test c.test' check
count_test_results total=5 pass=1 fail=0 xpass=0 xfail=0 skip=1 error=3

LC_ALL=C sort exp > t
mv -f t exp

# We need the sort below to account for parallel make usage.
grep ': [abcde]\.test' stdout \
  | sed "s/[ $tab]*#[ $tab]*SKIP.*//" \
  | LC_ALL=C sort > got

cat exp
cat got
diff exp got

:
