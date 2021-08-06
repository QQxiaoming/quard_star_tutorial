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

# TAP support: more unusual forms for valid TAP input.
# See also related test 'tap-fancy.sh'.

. test-init.sh

. tap-setup.sh

#
# From manpage Test::Harness::TAP(3):
#
#   Lines written to standard output matching /^(not )?ok\b/ must be
#   interpreted as test lines. All other lines must not be considered
#   test output.
#
# Unfortunately, the exact format of TODO and SKIP directives is not as
# clearly described in that manpage; but a simple reverse-engineering of
# the prove(1) utility shows that it is probably given by the perl regex
# /#\s*(TODO|SKIP)\b/.
#

# To avoid problems with backslashes in echo arguments.
xecho () { printf '%s\n' "$*"; }

# There are 34 values for $str ...
for str in \
  \'  \
  '"' \
  '`' \
  '#' \
  '$' \
  '!' \
  '\' \
  '/' \
  '&' \
  '%' \
  '(' \
  ')' \
  '|' \
  '^' \
  '~' \
  '?' \
  '*' \
  '+' \
  '-' \
  ',' \
  ':' \
  ';' \
  '=' \
  '<' \
  '>' \
  '@' \
  '[' \
  ']' \
  '{' \
  '}' \
  '\\' \
  '...' \
  '?[a-zA-Z0-9]*' \
  '*.*' \
; do
  # ... each of them add 1 pass, 1 fail, ...
  xecho "ok${str}"
  xecho "not ok${str}"
  # ... and (generally) 4 skips, 4 xfails, and 4 xpasses ...
  for settings in \
    'result="ok" directive=SKIP' \
    'result="not ok" directive=TODO' \
    'result="ok" directive=TODO' \
  ; do
    eval "$settings"
    xecho "${result}# ${directive}${str}"
    # ... but 6 skips, 6 xpasses and 6 xfails are to be removed, since
    # they might not work with $str = '#' or $str = '\' ...
    if test x"$str" != x'#' && test x"$str" != x'\'; then
      xecho "${result}${str}#${directive}"
      xecho "${result}${str}#   ${tab}${tab} ${directive}"
      xecho "${result}${str}#${directive}${str}"
    fi
  done
done > all.test

# Sanity check against a previous use of unportable usages of backslashes
# with the "echo" builtin.
if grep '[^\\]\\#' all.test; then
  framework_failure_ "writing backslashes in all.test"
fi

# ... so that we finally have:
pass=34
fail=34
xfail=130 # = 4 * 34 - 6
xpass=130 # = 4 * 34 - 6
skip=130  # = 4 * 34 - 6
error=0
total=$(($pass + $fail + $xfail + $xpass + $skip))

# Even nastier!  But accordingly to the specifics, it should still work.
for result in 'ok' 'not ok'; do
  echo "${result}{[(<#${tab}TODO>)]}" >> all.test
done
echo "ok{[(<#${tab}SKIP>)]}" >> all.test

# We have to update some test counts.
xfail=$(($xfail + 1))
xpass=$(($xpass + 1))
skip=$(($skip + 1))
total=$(($total + 3))

# And add the test plan!
echo 1..$total >> all.test

run_make -O -e FAIL check

$EGREP '^(PASS|FAIL|SKIP).*#.*TODO' stdout && exit 1
$EGREP '^X?(PASS|FAIL).*#.*SKIP' stdout && exit 1

count_test_results total=$total pass=$pass fail=$fail skip=$skip \
                   xpass=$xpass xfail=$xfail error=$error

:
