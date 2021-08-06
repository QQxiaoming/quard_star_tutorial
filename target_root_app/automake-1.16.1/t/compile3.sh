#! /bin/sh
# Copyright (C) 2010-2018 Free Software Foundation, Inc.
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

# Make sure 'compile' wraps the Microsoft C/C++ compiler (cl) correctly

am_create_testdir=empty
required=xsi-lib-shell
. test-init.sh

get_shell_script compile

# Use a dummy cl, since cl isn't readily available on all systems
cat >cl <<'END'
#! /bin/sh
echo "$@"
END

chmod +x ./cl

# POSIX mandates that the compiler accepts a space between the -I,
# -l and -L options and their respective arguments.  Traditionally,
# this should work also without a space.  Try both usages.
for sp in '' ' '; do

  # Check if compile handles "-o foo", -I, -l, -L, -Xlinker -Wl,
  opts=$(LIB= ./compile ./cl foo.c -o foo -l${sp}bar -L${sp}gazonk \
                             -I${sp}baz -Xlinker foobar -Wl,-foo,bar)
  test x"$opts" = \
       x"foo.c -Fefoo bar.lib -Ibaz -link -LIBPATH:gazonk foobar -foo bar"

  # Check if compile handles "-o foo.obj"
  opts=$(./compile ./cl -c foo.c -o foo.obj -I${sp}baz)
  test x"$opts" = x"-c foo.c -Fofoo.obj -Ibaz"

  # Check if compile handles "-o foo.o"
  opts=$(./compile ./cl -c foo.c -o foo.o -I${sp}baz)
  test x"$opts" = x"-c foo.c -Fofoo.o -Ibaz"

  # Check if compile handles "foo.cc" as C++.
  opts=$(./compile ./cl -c foo.cc -o foo.o -I${sp}baz)
  test x"$opts" = x"-c -Tpfoo.cc -Fofoo.o -Ibaz"

  # Check if compile clears the "eat" variable properly.
  opts=$(eat=1 ./compile ./cl -c foo.c -o foo.obj -I${sp}baz)
  test x"$opts" = x"-c foo.c -Fofoo.obj -Ibaz"

done

:
