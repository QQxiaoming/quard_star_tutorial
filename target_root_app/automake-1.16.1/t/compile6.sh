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

# Make sure 'compile' searches libraries correctly

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

  rm -rf lib lib2 syslib "sys  lib2"

  mkdir syslib
  : > syslib/foo.lib

  syslib=$(pwd)/syslib
  LIB=$syslib
  export LIB

  mkdir lib
  : > lib/bar.lib
  : > lib/bar.dll.lib
  : > lib/libbar.a
  : > lib/libbaz.a

  # Check if compile library search correctly
  opts=$(./compile ./cl foo.c -o foo -L${sp}lib -l${sp}bar -l${sp}foo)
  test x"$opts" = x"foo.c -Fefoo lib/bar.dll.lib $syslib/foo.lib -link -LIBPATH:lib"

  # Check if -static makes compile avoid bar.dll.lib
  opts=$(./compile ./cl foo.c -o foo -L${sp}lib -static -l${sp}bar -l${sp}foo)
  test x"$opts" = x"foo.c -Fefoo lib/bar.lib $syslib/foo.lib -link -LIBPATH:lib"

  : > syslib/bar.lib
  : > syslib/bar.dll.lib
  : > syslib/libbar.a

  # Check if compile finds bar.dll.lib in syslib
  opts=$(./compile ./cl foo.c -o foo -l${sp}bar -l${sp}foo)
  test x"$opts" = x"foo.c -Fefoo $syslib/bar.dll.lib $syslib/foo.lib"

  # Check if compile prefers -L over $LIB
  opts=$(./compile ./cl foo.c -o foo -L${sp}lib -l${sp}bar -l${sp}foo)
  test x"$opts" = x"foo.c -Fefoo lib/bar.dll.lib $syslib/foo.lib -link -LIBPATH:lib"

  # Check if compile falls back to finding classic libname.a style libraries
  # when name.lib or name.dll.lib isn't available.
  opts=$(./compile ./cl foo.c -o foo -L${sp}lib -l${sp}baz)
  test x"$opts" = x"foo.c -Fefoo lib/libbaz.a -link -LIBPATH:lib"

  mkdir lib2
  : > lib2/bar.dll.lib

  # Check if compile avoids bar.dll.lib in lib2 when -static
  opts=$(./compile ./cl foo.c -o foo -L${sp}lib2 -static -l${sp}bar -l${sp}foo)
  test x"$opts" = x"foo.c -Fefoo $syslib/bar.lib $syslib/foo.lib -link -LIBPATH:lib2"

  # Check if compile gets two different bar libraries when -static
  # is added in the middle
  opts=$(./compile ./cl foo.c -o foo -L${sp}lib2 -L${sp}lib -l${sp}bar -static -l${sp}bar)
  test x"$opts" = x"foo.c -Fefoo lib2/bar.dll.lib lib/bar.lib -link -LIBPATH:lib2 -LIBPATH:lib"

  # Check if compile gets the correct bar.dll.lib
  opts=$(./compile ./cl foo.c -o foo -L${sp}lib -L${sp}lib2 -l${sp}bar -l${sp}foo)
  test x"$opts" = x"foo.c -Fefoo lib/bar.dll.lib $syslib/foo.lib -link -LIBPATH:lib -LIBPATH:lib2"

  # Check if compile gets the correct bar.dll.lib
  opts=$(./compile ./cl foo.c -o foo -L${sp}lib2 -L${sp}lib -l${sp}bar -l${sp}foo)
  test x"$opts" = x"foo.c -Fefoo lib2/bar.dll.lib $syslib/foo.lib -link -LIBPATH:lib2 -LIBPATH:lib"

  mkdir "sys  lib2"
  : > "sys  lib2/foo.dll.lib"

  syslib2="$(pwd)/sys  lib2"
  LIB="$syslib2;$LIB"

  # Check if compile handles spaces in $LIB and that it obeys the order
  # in a multi-component $LIB.
  opts=$(./compile ./cl foo.c -o foo -l${sp}foo)
  test x"$opts" = x"foo.c -Fefoo $syslib2/foo.dll.lib"

  # Check if compile handles the 2nd directory in a multi-component $LIB.
  opts=$(./compile ./cl foo.c -o foo -static -l${sp}foo)
  test x"$opts" = x"foo.c -Fefoo $syslib/foo.lib"

done

:
