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
#  - don't spuriously recognize lines that are "almost" TAP lines as
#    real TAP lines

. test-init.sh

. tap-setup.sh

echo 1..5 > all.test

# The only recognized directives are "TODO" and "SKIP".
# So the following should count as passed tests.
cat >> all.test <<END
ok 1 # XFAIL
ok 2 # SKIPPED
ok 3 # TO DO
ok 4 # TODOALL
ok 5 # FIXME
END

# According to documentation of Test::Harness::TAP(3):
#
#    Lines written to standard output matching /^(not )?ok\b/
#    must be interpreted as test lines. All other lines must
#    not be considered test output.

cat >> all.test <<END
 ok
 ok 1
${tab}ok
${tab}ok 1
 not ok
 not ok 1
${tab}not ok
${tab}not ok 1
notok
notok 1
not${tab}ok
not${tab}ok 1
not  ok
not  ok 1
no ok
no ok 1
# ok
# not ok
# ok 1
# not ok 1
#ok
#not ok
#ok 1
#not ok 1
END

set +x # Don't pollute logs too much.
for r in 'ok' 'not ok'; do
  for s1 in \
    a b c d e f g h i j k l m n o p q r s t u v w x y z \
    A B C D E F G H I J K L M N O P Q R S T U V W X Y Z \
    0 1 2 3 4 5 6 7 8 9 _ ab 0a 23 a1B2c _o _x_y_
  do
    for s2 in '' @ % + - = / . : \; \* \? \& \! \# \$ \< \> \\; do
      printf '%s\n' "$r$s1$s2"
    done
  done
done >> all.test
set -x # Reset shell xtraces.

# The prove(1) utility doesn't bail out on these, so our driver
# shouldn't either.
# See comments in 'tap-bailout-leading-space.sh' for an explanation
# of why we don't have a whitespace-prepended "Bail out!" line here.
cat >> all.test <<'END'
bailout
bailout!
bail out
bail out!
Bailout
Bailout!
Bail out
 Bail out
#Bail out!
# Bail out!
END


# Debugging info and minor sanity check.
cat all.test \
  && test $(grep -c '^ok1$' all.test) -eq 1 \
  && test $(grep -c '^not ok1$' all.test) -eq 1 \
  || framework_failure_ "creating all.test"

run_make -O check
count_test_results total=5 pass=5 fail=0 xpass=0 xfail=0 skip=0 error=0

:
