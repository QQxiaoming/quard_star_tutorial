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
#  - all input (valid TAP lines, invalid TAP lines, non-TAP lines)
#    are passed through in the log file
#  - TAP errors are reported in the log file too
# See also related test 'tap-passthrough-exit.sh'.

. test-init.sh

weirdchars=\''"\$@!&()[]<>#;,:.^?*/'

. tap-setup.sh

#
# Only successful tests.
#

# The whitespace in this test might be normalized in the testsuite
# progress output, but should be copied verbatim in the log files.
cat > ok.test <<END
1..6
TAP plan in the previous line.
ok${tab}
ok     2
ok - foo
ok 4 - x
  This is not a TAP line, but should still be copied in the log file!
# some diagnostic${tab}
not ok # TODO low priority
ok # SKIP who cares?
$weirdchars
END

run_make TESTS=ok.test check || { cat ok.log; exit 1; }
cat ok.log

for rx in \
  '1\.\.6' \
  'TAP plan in the previous line\.' \
  "ok${tab}" \
  'ok     2' \
  'ok - foo' \
  'ok 4 - x' \
  '  This is not a TAP line, but should still be copied in the log file!' \
  "# some diagnostic${tab}" \
  'not ok # TODO low priority' \
  'ok # SKIP who cares?' \
; do
  grep "^$rx$" ok.log
done
$FGREP "$weirdchars" ok.log

#
# Mixed failing/successful tests.
#

cat > tiny.test <<END
1..1
ok
END

cat > ok.test <<END
1..1
ok
only one success here
END

cat > ko.test <<END
1..5
foo foo foo
ok${tab}
ok     2
not ok - foo
not ok 4 - x
# diagnostic ko
  bar${tab}bar${tab}bar
ok # TODO dunno
$weirdchars
END

cat > bail.test <<END
Bail out! Test is taking too long!
END

cat > skip.test <<END
1..0 # Skipped: WWW::Mechanize not installed
END

cat > err.test <<END
1..3
ok 1
Invalid test count
ok 23
Misplaced plan
1..13
ok
Extra test
ok
Last line
END

st=0
run_make check \
  TESTS='tiny.test ok.test ko.test bail.test skip.test err.test' || st=$?
cat tiny.log
cat ok.log
cat ko.log
cat bail.log
cat skip.log
cat err.log
test $st -gt 0 || exit 1

grep '^1\.\.1$' tiny.log
grep '^ok$' tiny.log
grep '^only one success here$' ok.log

for rx in \
  '1\.\.5' \
  'foo foo foo' \
  "ok${tab}" \
  'ok     2' \
  'not ok - foo' \
  'not ok 4 - x' \
  '# diagnostic ko' \
  "  bar${tab}bar${tab}bar" \
  'ok # TODO dunno' \
; do
  grep "^$rx$" ko.log
done
$FGREP "$weirdchars" ko.log

grep '^Bail out! Test is taking too long!$' bail.log
grep '^1\.\.0 # Skipped: WWW::Mechanize not installed$' skip.log

for rx in \
  '^1\.\.3$' \
  '^Invalid test count$' \
  '^ok 23$' \
  '^Misplaced plan$' \
  '^1\.\.13$' \
  '^ERROR:.* multiple test plans' \
   '^Extra test$' \
  '^Last line$' \
  '^ERROR:.* [tT]oo many tests run.*expected 3, got 4' \
  '^ERROR:.* err\.test 23 .*OUT[ -]OF[ -]ORDER.*expecting 2' \
; do
  grep "$rx" err.log
done

:
