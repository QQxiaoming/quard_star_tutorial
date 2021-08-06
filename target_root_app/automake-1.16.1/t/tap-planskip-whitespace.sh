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
#    with a SKIP directive in the TAP plan

. test-init.sh

. tap-setup.sh

cat > foo.test <<END
1..0${tab}${tab}   #${tab}SKIP  ${tab}Strip leading & trailing ${tab}${tab}
END

cat > bar.test <<END
1..0 #SKIP Preserve ${tab}  middle${tab}${tab}space
END

cat > baz.test <<END
1..0 #  SKIP${tab}  Strip${tab}external  preserve ${tab}middle  ${tab}${sp}
END

run_make -O TESTS='foo.test bar.test baz.test' check
count_test_results total=3 pass=0 fail=0 error=0 xpass=0 xfail=0 skip=3

grep "SKIP: foo\\.test - Strip leading & trailing$" stdout
grep "SKIP: bar\\.test - Preserve ${tab}  middle${tab}${tab}space" stdout
grep "SKIP: baz\\.test - Strip${tab}external  preserve ${tab}middle" stdout

:
