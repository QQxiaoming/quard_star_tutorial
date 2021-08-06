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
#  - diagnostic messages (TAP lines with leading "#")
#  - flags '--comments' and '--no-comments' of the TAP test driver

. test-init.sh

. tap-setup.sh

metacharacters=\''"\$!&()[]<>#;^?*'

cat > all.test <<END
1..4
# Hi! I'm a comment.
# Tests begin.
ok 1
not ok 2 - foo # TODO
ok 3 - bar # SKIP
# Tests end.
ok - zardoz
# Shell metacharacters here: $metacharacters
.# Leading characters before "#", not a TAP diagnostic line.
x # Leading characters before "#", not a TAP diagnostic line.
 # Leading whitespace before "#", not a TAP diagnostic line.
${tab}# Leading whitespace before "#", not a TAP diagnostic line.
 ${tab}  # Leading whitespace before "#", not a TAP diagnostic line.
END

cat > exp <<END
# all.test: Hi! I'm a comment.
# all.test: Tests begin.
PASS: all.test 1
XFAIL: all.test 2 - foo # TODO
SKIP: all.test 3 - bar # SKIP
# all.test: Tests end.
PASS: all.test 4 - zardoz
# all.test: Shell metacharacters here: $metacharacters
END

run_make -O check
$EGREP -i "#.*all\\.test|a comment|(Tests|Shell) " stdout && exit 1
count_test_results total=4 pass=2 fail=0 xpass=0 xfail=1 skip=1 error=0

echo 'AM_TEST_LOG_DRIVER_FLAGS = --comments' >> Makefile
run_make -O check
$FGREP ' all.test' stdout > got
cat exp
cat got
diff exp got
count_test_results total=4 pass=2 fail=0 xpass=0 xfail=1 skip=1 error=0

run_make -O TEST_LOG_DRIVER_FLAGS="--no-comments" check
$EGREP -i "#.*all\\.test|a comment|(Tests|Shell) " stdout && exit 1
count_test_results total=4 pass=2 fail=0 xpass=0 xfail=1 skip=1 error=0

# The "#"-prepended lines here shouldn't be parsed as test results.
cat > all.test <<END
1..1
ok
# ok
#ok
# not ok
#not ok
# Bail out!
#Bail out!
# SKIP
#SKIP
# TODO
#TODO
END

run_make -O check
count_test_results total=1 pass=1 fail=0 xpass=0 xfail=0 skip=0 error=0

# Diagnostic without leading whitespace, or with extra leading whitespace,
# is ok.  Be laxer in the grepping checks, to allow for whitespace
# normalization by the TAP driver.

ws="[ $tab]"
ws0p="${ws}*"
ws1p="${ws}${ws0p}"

cat > all.test <<END
1..1
ok 1
#foo
#bar${tab}
#   zardoz  ${tab}
#  ${tab} ${tab}${tab}foo  bar${tab}baz  ${tab}
END

run_make -O check
count_test_results total=1 pass=1 fail=0 xpass=0 xfail=0 skip=0 error=0

grep "^# all.test:${ws0p}foo$" stdout
grep "^# all.test:${ws0p}bar${ws0p}$" stdout
grep "^# all.test:${ws1p}zardoz${ws0p}$" stdout
grep "^# all.test:${ws1p}foo  bar${tab}baz${ws0p}$" stdout

test $(grep -c '^# all\.test:' stdout) -eq 4

:
