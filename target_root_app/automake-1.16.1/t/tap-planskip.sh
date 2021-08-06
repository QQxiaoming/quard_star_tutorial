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
#  - special plan format to skip all the tests in a script

. test-init.sh

. tap-setup.sh

weirdchars=\''"$!&()[]<>#;^?*/@%=,.:'

cat > foo.test <<END
1..0
END

cat > bar.test <<END
blah
# blah
1..0$tab $tab
END

# It is undefined whether the comment after the plan below should
# count as an explanation; still, the test should be skipped.
cat > baz.test <<END
1..0 # WWW::Mechanize not installed
other
    junk
       lines
END

cat > wget.test <<END
1..0 # SKIP wget(1) not installed
# See also curl.test
END

cat > curl.test <<END
1..0 # SKIP: Can't connect to gnu.org!
# See also wget.test
END

cat > mu.test <<END
1..0 # SKIP $weirdchars
END

run_make -O check \
  TESTS='foo.test bar.test baz.test wget.test curl.test mu.test'
count_test_results total=6 pass=0 fail=0 xpass=0 xfail=0 skip=6 error=0

# Look for a regression where the "1..0" wasn't being stripped from the
# SKIP message.
$FGREP '1..0' stdout && exit 1

grep '^SKIP: foo\.test$' stdout
grep '^SKIP: bar\.test$' stdout
grep '^SKIP: baz\.test' stdout # Deliberately laxer, see above for why.
grep '^SKIP: wget\.test .* wget(1) not installed$' stdout
grep '^SKIP: curl\.test .* Can'\''t connect to gnu\.org!$' stdout
grep '^SKIP: mu\.test' stdout | $FGREP "$weirdchars" stdout
test $(grep -c ': .*\.test' stdout) -eq 6

:
