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

# Unit test on TAP driver:
#  - error messages from awk/shell/perl goes to the console

required=non-root
am_create_testdir=empty
. test-init.sh

fetch_tap_driver

tst=zardoz

for suf in trs log; do

  rm -f $tst.log $tst.trs
  touch $tst.$suf
  chmod a-w $tst.$suf

  st=0
  ./tap-driver --test-name $tst --log-file $tst.log --trs-file $tst.trs \
    -- sh -c 'echo 1..1; echo ok 1; echo "Hello, World!"' \
    >stdout 2>stderr && st=1
  cat stdout
  cat stderr >&2
  cat $tst.log || :
  cat $tst.trs || :
  test $st -eq 0

  $FGREP 'Hello, World!' stderr stdout && exit 1
  $FGREP $tst.$suf stderr

done

:
