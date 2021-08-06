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

# TAP support:
#  - test plan preceding and/or following non-result TAP lines

. test-init.sh

. tap-setup.sh

cat > top1.test <<END
non-TAP line, ignored
1..1
ok 1
END

cat > top2.test <<END
some
non-TAP
lines
are
ignored
# and a TAP comment won't cause problems either
1..2
ok 1
ok 2
END

# Try with a blank line too, just to be sure.
cat > top3.test <<END

1..1
ok 1
END

cat > bot1.test <<END
ok 1 # SKIP
1..1
bla blah blah ...
END

cat > bot2.test <<END
ok 1
ok 2
not ok 3 # TODO
1..3
#@$%! (a cursing comment :-)
END

# Try with a blank line too, just to be sure.
cat > bot3.test <<END
ok 1
not ok 2 # TODO
ok 3 # SKIP
ok 4 # SKIP
1..4

END

tests=$(echo *.test)

for tap_flags in "" "--comments"; do
  run_make -O TEST_LOG_DRIVER_FLAGS="$tap_flags" TESTS="$tests" check
  count_test_results total=12 pass=7 xfail=2 skip=3 fail=0 xpass=0 error=0
done

:
