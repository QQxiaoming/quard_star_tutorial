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
#  - dependencies between test scripts

. test-init.sh

cat > Makefile.am << 'END'
# The tests are *deliberately* listed in inversed order here.
TESTS = c.test b.test a.test
b.log: a.log
c.log: b.log
END

. tap-setup.sh

cat > a.test << 'END'
#!/bin/sh
echo 1..2
echo ok 1
# Creative quoting below to please maintainer-check.
sleep '3'
echo ok 2
: > a.run
END

cat > b.test << 'END'
#!/bin/sh
echo 1..2
if test -f a.run; then
  echo ok 1
else
  echo not ok 1
fi
# Creative quoting below to please maintainer-check.
sleep '3'
echo ok 2
: > b.run
END

cat > c.test << 'END'
#!/bin/sh
echo 1..1
test -f b.run || { echo 'Bail out!'; exit 1; }
echo ok 1
rm -f a.run b.run
END

chmod a+x *.test

run_make -O check
count_test_results total=5 pass=5 fail=0 xpass=0 xfail=0 skip=0 error=0

cat > exp << 'END'
PASS: a.test 1
PASS: a.test 2
PASS: b.test 1
PASS: b.test 2
PASS: c.test 1
END

grep ': [abc]\.test' stdout > got

cat exp
cat got
diff exp got

# TODO: it would be nice to also redo the checks forcing parallel make...

:
