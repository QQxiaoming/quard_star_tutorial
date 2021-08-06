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

# TAP support: the following situations should be flagged as errors:
#  - empty TAP input
#  - blank TAP input

. test-init.sh

. tap-setup.sh

# Empty TAP input.
: > empty.test

# Blank TAP input (one empty line).
echo > blank.test

# Blank TAP input (one whitespace-only line).
cat > white.test  <<END
  ${tab}
END

# Blank TAP input (few blank and whitespace-only lines).
cat > white2.test  <<END


 ${tab}  ${tab}${tab}

${tab}

END

for input in empty blank white white2; do
  cp $input.test all.test
  run_make -O -e FAIL check
  count_test_results total=1 pass=0 fail=0 xpass=0 xfail=0 skip=0 error=1
  grep '^ERROR: all\.test - missing test plan$' stdout
done

:
