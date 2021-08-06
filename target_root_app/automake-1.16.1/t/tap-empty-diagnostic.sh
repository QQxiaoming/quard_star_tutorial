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
#  - empty diagnostic messages are discarder

. test-init.sh

. tap-setup.sh

sed 's/\$$//' > all.test <<END
1..1$
ok 1$
#$
# $
#${tab}$
#   ${tab} $tab${tab}$
END

run_make -O check

grep '^PASS:.*all\.test' stdout # Sanity check.
grep '#.*all\.test' stdout && exit 1
grep "all\.test[ $tab]*:[ $tab]*$" stdout && exit 1

:
