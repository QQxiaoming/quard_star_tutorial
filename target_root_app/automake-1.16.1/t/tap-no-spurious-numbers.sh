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
#  - we shouldn't spuriously recognize as TAP result numbers what it
#    not, even if it seems pretty close

. test-init.sh

. tap-setup.sh

cat > prefixes <<'END'
A
a
_
+
-
=
/
*
.
:
,
;
$
@
%
&
#
?
!
|
\
"
`
'
(
)
[
]
{
}
<
>
END

n=$(wc -l <prefixes)

# See the loop below to understand this initialization.
pass=$(($n * 3))
fail=$pass
skip=$(($pass - 3))
xfail=$skip
xpass=$xfail
error=0
total=$(($pass + $fail + $skip + $xfail + $xpass))

echo 1..$total > all.test

highno=1000

for result in 'ok' 'not ok'; do
  for spacing in "" " " "$tab"; do
    subst="$result &$spacing$higno"
    sed -e "s|.*|$subst|" prefixes
    for directive in TODO SKIP; do
      test "$result $directive" != "not ok SKIP" || continue
      sed -e '/^#$/d' -e "s|.*|$subst # $directive|" prefixes
    done
  done
done >> all.test

cat all.test # For debugging.

# Sanity checks.
grep '#.*#' all.test \
  && framework_failure_ "creating all.test"
test $(wc -l <all.test) -lt $highno \
  || framework_failure_ "creating all.test"

run_make -O -e IGNORE check
count_test_results total=$total pass=$pass fail=$fail skip=$skip \
                   xpass=$xpass xfail=$xfail error=$error

:
