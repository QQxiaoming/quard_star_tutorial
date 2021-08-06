#! /bin/sh
# Copyright (C) 2006-2018 Free Software Foundation, Inc.
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

# More install-sh checks: check -C.

am_create_testdir=empty
required=non-root
. test-init.sh

# Solaris /usr/ucb/touch does not accept -t.
touch -t "$old_timestamp" foo \
  || skip_ "touch utility doesn't accept '-t' option"

get_shell_script install-sh

./install-sh -d d1

# Do not change the timestamps when using -C.
echo foo >file
./install-sh -C file d1
TZ=UTC0 touch -t $old_timestamp d1/file
./install-sh -C file d1
is_newest file d1/file
echo foo1 >file
./install-sh -C file d1
diff file d1/file
# Rights must be updated.
./install-sh -C -m 444 file d1
test -r d1/file
test ! -w d1/file

:
