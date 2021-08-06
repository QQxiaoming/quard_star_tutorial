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
#  - plan line with trailing whitespace is recognized and handled correctly

. test-init.sh

. tap-setup.sh

sed 's/\$$//' > foo.test <<END
1..2 $
ok 1$
ok 2$
END

cat > bar.test <<END
1..1$tab   $tab$tab
ok 1
END

run_make -O TESTS='foo.test bar.test' check
count_test_results total=3 pass=3 fail=0 error=0 xpass=0 xfail=0 skip=0

:
