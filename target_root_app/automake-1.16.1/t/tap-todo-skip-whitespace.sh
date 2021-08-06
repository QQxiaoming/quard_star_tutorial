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
#  - normalization of whitespace in console testsuite progress associated
#    with TODO and SKIP directives

. test-init.sh

. tap-setup.sh

cat > stub.tap <<END
1 # TODO
2 # TODO foo?
3 # TODO: bar!
4 aa # TODO
5 bb # TODO       fnord 5
6 cc # TODO:${tab}fnord 6
7 - x # TODO
8 - y # TODO fnord $tab  8
9 - z # TODO: fnord 9  $tab
10# TODO x0
11$tab# TODO x1
12 $tab$tab   # TODO x2
13 asd# TODO x3
14 sad$tab# TODO x4
15 das$tab$tab   # TODO x5
END

cat > stub.exp <<END
1 # TODO
2 # TODO foo?
3 # TODO: bar!
4 aa # TODO
5 bb # TODO fnord 5
6 cc # TODO:${tab}fnord 6
7 - x # TODO
8 - y # TODO fnord $tab  8
9 - z # TODO: fnord 9
10 # TODO x0
11 # TODO x1
12 # TODO x2
13 asd # TODO x3
14 sad # TODO x4
15 das # TODO x5
END

plan=1..15

my_make_check ()
{
  xpass=0 xfail=0 skip=0
  case $1 in
    xpass|xfail|skip) eval $1=15;;
    *) fatal_ "bad argument '$1' for my_make_check";;
  esac
  cat all.test
  # We don't care about the exit status in this test.
  run_make -O -e IGNORE check
  count_test_results total=15 pass=0 fail=0 error=0 \
                     xpass=$xpass xfail=$xfail skip=$skip
  # Don't be too strict w.r.t. possible normalization of "TODO: foo" into
  # "TODO : foo" (as is done by, e.g., the 'TAP::Parser' perl module).
  LC_ALL=C grep '^[A-Z][A-Z]*:' stdout \
    | sed -e 's/# TODO *:/# TODO:/' -e 's/# SKIP *:/# SKIP:/' > got
  cat exp
  cat got
  diff exp got
}

# For "TODO" directives leading to XPASS results.
(echo $plan && sed -e 's/^/ok /' stub.tap) > all.test
sed -e 's/^/XPASS: all.test /' stub.exp > exp
my_make_check xpass

# For "TODO" directives leading to XFAIL results.
(echo $plan && sed -e 's/^/not ok /' stub.tap) > all.test
sed -e 's/^/XFAIL: all.test /' stub.exp > exp
my_make_check xfail

# For "SKIP" directives.
(echo $plan && sed -e 's/^/ok /' -e 's/TODO/SKIP/' stub.tap) > all.test
sed -e 's/TODO/SKIP/' -e 's/^/SKIP: all.test /' stub.exp > exp
my_make_check skip

:
