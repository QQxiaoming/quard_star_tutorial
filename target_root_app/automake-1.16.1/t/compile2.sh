#! /bin/sh
# Copyright (C) 2009-2018 Free Software Foundation, Inc.
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

# Make sure 'compile' deals correctly with Windows-style paths.

am_create_testdir=empty
. test-init.sh

get_shell_script compile

cat >mycc <<'END'
source_seen=no
for arg
do
  test "X$arg" = X-o && exit 1
  test "X$arg" = "X$amtest_source" && source_seen=yes
done
if test "$source_seen" != yes; then
  echo "$0: no source file seen" >&2
  exit 1
fi
if test ! -f "$amtest_source"; then
  echo "$0: $amtest_source not readable" >&2
  exit 1
fi
if test ! -d "$amtest_lock"; then
  echo "$0: no lockdir $amtest_lock" >&2
  exit 1
fi
touch "$amtest_obj"
END

chmod +x ./mycc

# In case this test runs on a system with backslash directory separators:
mkdir libltdl libltdl/libltdl

# Backslashes in the input and the output name should be accepted.
# Since this test might run on non-w32 systems, we need to be careful not
# to use any backslash sequences which might be interpreted by 'echo'.
amtest_source='libltdl\libltdl\slist.c'
amtest_object='libltdl\libltdl\libltdl_libltdl_la-slist.obj'
amtest_obj='slist.o'
amtest_lock='slist_o.d'
export amtest_source amtest_object amtest_obj amtest_lock

: > "$amtest_source"
./compile ./mycc -c "$amtest_source" -o "$amtest_object"
test -f "$amtest_object"

# Absolute Windows paths should be accepted.
# Do not actually run this test on anything that could be Windows.
if test -d 'C:\'; then
  skip_ "this test shouldn't run on a Windows-like system"
fi
case $PATH_SEPARATOR in
 ';'|':');;
 *) skip_ "unrecognized PATH separator '$PATH_SEPARATOR'"
esac

amtest_source='C:\libltdl\libltdl\slist.c'
amtest_object='C:\libltdl\libltdl\libltdl_libltdl_la-slist.obj'
amtest_obj='slist.o'
amtest_lock='slist_o.d'
export amtest_source amtest_object amtest_obj amtest_lock

: > "$amtest_source"
./compile ./mycc -c "$amtest_source" -o "$amtest_object"
test -f "$amtest_object"

:
