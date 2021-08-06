#! /bin/sh
# Copyright (C) 2004-2018 Free Software Foundation, Inc.
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

# Test missing when running a tool's --version.

am_create_testdir=empty
. test-init.sh

get_shell_script missing

# b7cb8259 assumed not to exist.

run_cmd ()
{
  st=0; "$@" >stdout 2>stderr || st=$?
  cat stdout
  cat stderr >&2
  return $st
}

run_cmd ./missing b7cb8259 --version && exit 1
grep WARNING stderr && exit 1
run_cmd ./missing b7cb8259 --grep && exit 1

if test x"$am_test_prefer_config_shell" != x"yes"; then
  # The /bin/sh from Solaris 10 is a spectacular failure.  After a failure
  # due to a "command not found", it sets '$?' to '1'.
  if (st=0; /bin/sh -c 'no--such--command' || st=$?; test $st -eq 127); then
    grep 'WARNING:.*missing on your system' stderr
  fi
fi

# missing itself it known to exist :)

run_cmd ./missing ./missing --version || exit 1
grep 'missing .*(GNU [aA]utomake)' stdout
test -s stderr && exit 1
run_cmd ./missing ./missing --grep && exit 1
grep WARNING stderr && exit 1
grep "missing:.* unknown '--grep'" stderr

:
