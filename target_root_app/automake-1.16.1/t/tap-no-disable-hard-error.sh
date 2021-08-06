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
#  - "Bail out!" magic and TAP parse errors are not disabled nor turned
#    into simple failures by the definition DISABLE_HARD_ERRORS.

. test-init.sh

cat > Makefile.am << 'END'
DISABLE_HARD_ERRORS = yes
TEST_LOG_COMPILER = cat
TESTS = bail.test few.test noplan.test
END

. tap-setup.sh

cat > bail.test <<END
1..1
Bail out!
END

cat > few.test <<END
1..1
END

cat > noplan.test <<END
# nothing here
END

run_make -O -e FAIL check
count_test_results total=3 pass=0 fail=0 xpass=0 xfail=0 skip=0 error=3

:
