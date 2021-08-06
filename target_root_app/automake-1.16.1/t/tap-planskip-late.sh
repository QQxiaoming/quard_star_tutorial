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
#  - the special "plan with SKIP" can also be used "late" in the TAP
#    stream, i.e., preceded by non-TAP output or TAP diagnostic.

. test-init.sh

. tap-setup.sh

cat > foo.test <<END
some non-TAP text, will be copied in the global log
1..0 # SKIP from the last line
END

cat > bar.test <<END
# some TAP diagnostic, will go to console
1..0
END

run_make -O TESTS='foo.test bar.test' check

grep '^SKIP: foo\.test .* from the last line$' stdout
grep '^SKIP: bar\.test$' stdout
test $(grep -c ': .*\.test' stdout) -eq 2
count_test_results total=2 pass=0 fail=0 xpass=0 xfail=0 skip=2 error=0

:
