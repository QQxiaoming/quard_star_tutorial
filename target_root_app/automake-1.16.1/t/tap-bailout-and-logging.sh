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
#  - even after a "Bail out!" directive, all input is still copied in
#    the log file

. test-init.sh

. tap-setup.sh

cat > all.test <<END
First line
Bail out!
non-TAP line after bailout
# TAP diagnostic after bailout
1..0 # SKIP TAP plan after bailout
ok 1 - TAP result after bailout
END

$MAKE check && { cat all.log; exit 1; }
cat all.log

for rx in \
  'First line' \
  'Bail out!' \
  'non-TAP line after bailout' \
  '# TAP diagnostic after bailout' \
  '1\.\.0 # SKIP TAP plan after bailout' \
  'ok 1 - TAP result after bailout' \
; do
  grep "^$rx$" all.log
done

:
