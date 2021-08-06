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
#  - A "Bail out!" directive causes the driver to ignore the exit
#    status of the test script.

. test-init.sh

tests='exit.test exit127.test sighup.test sigterm.test'

cat > Makefile.am <<END
TESTS = $tests
END

. tap-setup.sh

cat > exit.test << 'END'
#!/bin/sh
echo 'Bail out!'
exit 1
END

cat > exit127.test << 'END'
#!/bin/sh
echo 'Bail out!'
exit 127
END

cat > sighup.test << 'END'
#!/bin/sh
echo 'Bail out!'
kill -1 $$
END

cat > sigterm.test << 'END'
#!/bin/sh
echo 'Bail out!'
kill -15 $$
END

chmod a+x $tests

run_make -O -e FAIL check
count_test_results total=4 pass=0 fail=0 xpass=0 xfail=0 skip=0 error=4

for tst in $tests; do grep "^ERROR: $tst - Bail out!" stdout; done
$EGREP "ERROR: .*(exit.*status|terminat.*signal)" stdout && exit 1

:
