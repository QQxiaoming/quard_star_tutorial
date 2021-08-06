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

# Basic TAP test protocol support:
#  - "Bail out!" magic

. test-init.sh

. tap-setup.sh

: > exp

#------------------------------------------------------------------

# Bailout without explanation.

cat > a.test <<END
1..4
ok 1
not ok 2
Bail out!
not ok 3
ok 4 # SKIP
END

cat >> exp <<END
PASS: a.test 1
FAIL: a.test 2
ERROR: a.test - Bail out!
END

# pass += 1, fail +=1, error += 1

#------------------------------------------------------------------

# Bailout with explanation.

cat > b.test <<END
1..7
ok 1 # SKIP
ok 2 # TODO
not ok 3 # TODO
Bail out! We're out of disk space.
ok 4
not ok 5
not ok 6 # TODO
ok 7 # TODO
END

cat >> exp <<END
SKIP: b.test 1 # SKIP
XPASS: b.test 2 # TODO
XFAIL: b.test 3 # TODO
ERROR: b.test - Bail out! We're out of disk space.
END

# skip += 1, xpass += 1, xfail += 1, error += 1

#------------------------------------------------------------------

# Bail out before the test plan.

cat > c.test <<END
ok 1
ok 2
Bail out! BOOOH!
1..2
END

cat >> exp <<END
PASS: c.test 1
PASS: c.test 2
ERROR: c.test - Bail out! BOOOH!
END

# pass += 2, error += 1

#------------------------------------------------------------------

# Bailout on the first line.

cat > d.test <<END
Bail out! mktemp -d: Permission denied
ok 1
END

echo 'ERROR: d.test - Bail out! mktemp -d: Permission denied' >> exp

# error += 1

#------------------------------------------------------------------

# TAP input comprised only of a bailout directive.

cat > e.test <<END
Bail out!
END

echo "ERROR: e.test - Bail out!" >> exp

# error += 1

#------------------------------------------------------------------

# Doing the sums above, we have:
test_counts='total=12 pass=3 fail=1 xpass=1 xfail=1 skip=1 error=5'

run_make -O -e FAIL TESTS='a.test b.test c.test d.test e.test' check
count_test_results $test_counts

LC_ALL=C sort exp > t
mv -f t exp

# We need the sort below to account for parallel make usage.
grep ': [abcde]\.test' stdout | LC_ALL=C sort > got

cat exp
cat got
diff exp got

:
