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
#  - A "Bail out!" directive causes the driver to ignore any TAP
#    diagnostic message in the rest of the following TAP stream.

. test-init.sh

. tap-setup.sh

echo AM_TEST_LOG_DRIVER_FLAGS = --comments >> Makefile

cat > all.test <<END
1..1
# sanity check
ok 1
Bail out!
# not seen
END

run_make -O -e FAIL check
count_test_results total=2 pass=1 fail=0 xpass=0 xfail=0 skip=0 error=1
grep '# all.test: sanity check' stdout
grep 'not seen' stdout && exit 1

:
