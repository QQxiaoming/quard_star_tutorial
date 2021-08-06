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
#  - TAP result numbers terminated by a non-whitespace "word boundary"
#    character are recognized

. test-init.sh

. tap-setup.sh

cat > all.test <<'END'
1..5
ok 1: foo1
not ok 2- foo2
ok 3@ foo3 # SKIP
not ok 4?&%$ foo4 # TODO
ok 5"`!! foo5 # TODO
END

run_make -O -e FAIL check
count_test_results total=5 pass=1 fail=1 skip=1 xpass=1 xfail=1 error=0

$EGREP '^PASS: all\.test 1 ?: foo1$' stdout
$EGREP '^FAIL: all\.test 2 ?- foo2$' stdout
$EGREP '^SKIP: all\.test 3 ?@ foo3 # SKIP$' stdout
$EGREP '^XFAIL: all\.test 4 ?\?&%\$ foo4 # TODO$' stdout
$EGREP '^XPASS: all\.test 5 ?"`!! foo5 # TODO$' stdout

cat > all.test <<'END'
1..3
ok 2: quux
not ok 1!
ok 476$ # SKIP
END

run_make -O -e FAIL check
count_test_results total=3 pass=0 fail=0 skip=0 xpass=0 xfail=0 error=3

$EGREP '^ERROR: all\.test 2 ?: quux # OUT-OF-ORDER \(expecting 1\)$' stdout
$EGREP '^ERROR: all\.test 1 ?! # OUT-OF-ORDER \(expecting 2\)$' stdout
$EGREP '^ERROR: all\.test 476 ?\$ # OUT-OF-ORDER \(expecting 3\)$' stdout

cat > punctuation <<'END'
'
"
`
#
$
!
\
/
&
%
(
)
|
^
~
?
*
+
-
,
:
;
=
<
>
@
[
]
{
}
\
END

# Strip any extra whitespace, for Solaris' wc.
planned=$(wc -l <punctuation | tr -d " $tab")

echo 1..$planned > all.test
awk '{print "ok " NR $0 }' punctuation >> all.test
# For debugging.
cat all.test
# Sanity check.
grep "^ok 1'$" all.test || framework_failure_ "creating all.test"

run_make -O check
count_test_results total=$planned pass=$planned \
                   fail=0 skip=0 xpass=0 xfail=0 error=0

:
