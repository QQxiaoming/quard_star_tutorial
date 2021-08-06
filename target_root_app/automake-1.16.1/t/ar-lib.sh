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

# Make sure 'ar-lib' wraps the Microsoft Library Manager (lib) correctly.

am_create_testdir=empty
required=xsi-lib-shell
. test-init.sh

get_shell_script ar-lib

# Use a dummy lib, since lib isn't readily available on all systems.
cat >lib <<'END'
#! /bin/sh
if test x"$2" = x-LIST && test x"$3" = xfake.lib; then
  echo fake.obj
elif test x"$2" = x-LIST && test x"$3" = xfake2.lib; then
  printf "%s\n" "dir\\fake2.obj"
else
  printf "%s\n" "lib $*"
fi
END

chmod +x ./lib

# Check if ar-lib can create an archive with "cr".
opts=$(./ar-lib ./lib cr foo.lib foo.obj)
test x"$opts" = x"lib -NOLOGO -OUT:foo.lib foo.obj"

# Check if ar-lib can update an existing archive with "r".
touch foo.lib
opts=$(./ar-lib ./lib r foo.lib foo.obj)
test x"$opts" = x"lib -NOLOGO -OUT:foo.lib foo.lib foo.obj"

# Check if ar-lib can update an existing archive with "q".
opts=$(./ar-lib ./lib q foo.lib foo.obj)
test x"$opts" = x"lib -NOLOGO -OUT:foo.lib foo.lib foo.obj"

# Check if ar-lib accepts "u" as a modifier.
# NOTE: "u" should have an effect, but currently doesn't.
opts=$(./ar-lib ./lib ru foo.lib foo.obj)
test x"$opts" = x"lib -NOLOGO -OUT:foo.lib foo.lib foo.obj"

# Check if ar-lib accepts "s" as a modifier.
opts=$(./ar-lib ./lib rs foo.lib foo.obj)
test x"$opts" = x"lib -NOLOGO -OUT:foo.lib foo.lib foo.obj"

# Check if ar-lib accepts "S" as a modifier.
opts=$(./ar-lib ./lib rS foo.lib foo.obj)
test x"$opts" = x"lib -NOLOGO -OUT:foo.lib foo.lib foo.obj"

# Check if ar-lib passes on @FILE with "r"
opts=$(./ar-lib ./lib r foo.lib @list)
test x"$opts" = x"lib -NOLOGO -OUT:foo.lib foo.lib @list"

# Check if ar-lib can delete a member from an archive with "d".
opts=$(./ar-lib ./lib d foo.lib foo.obj)
test x"$opts" = x"lib -NOLOGO -REMOVE:foo.obj foo.lib"

# Check if ar-lib can delete members in an @FILE.
echo foo.obj > foolist
opts=$(./ar-lib ./lib d foo.lib @foolist)
test x"$opts" = x"lib -NOLOGO -REMOVE:foo.obj foo.lib"

# Check if ar-lib can list archive members with "t".
opts=$(./ar-lib ./lib t foo.lib)
test x"$opts" = x"lib -NOLOGO -LIST foo.lib"

# Check if ar-lib accepts "v" as a modifier.
# NOTE: "v" should have an effect, but currently doesn't.
opts=$(./ar-lib ./lib tv foo.lib)
test x"$opts" = x"lib -NOLOGO -LIST foo.lib"

# Check if ar-lib can extract archive members with "x".
touch fake.lib
opts=$(./ar-lib ./lib x fake.lib)
test x"$opts" = x"lib -NOLOGO -EXTRACT:fake.obj fake.lib"

# Check if ar-lib can extract specified archive members with "x".
opts=$(./ar-lib ./lib x foo.lib foo.obj)
test x"$opts" = x"lib -NOLOGO -EXTRACT:foo.obj foo.lib"

# Check if ar-lib can extract members in an @FILE.
opts=$(./ar-lib ./lib x foo.lib @foolist)
test x"$opts" = x"lib -NOLOGO -EXTRACT:foo.obj foo.lib"

# Check if ar-lib passes -lib and -LTCG through to the wrappee.
opts=$(./ar-lib ./lib -lib -LTCG x foo.lib foo.obj)
test x"$opts" = x"lib -lib -LTCG -NOLOGO -EXTRACT:foo.obj foo.lib"

# Check if ar-lib can extract backslashed members.
touch fake2.lib
opts=$(./ar-lib ./lib x fake2.lib)
test x"$opts" = x"lib -NOLOGO -EXTRACT:dir\\fake2.obj fake2.lib"

# Check if ar-lib accepts "s" as an action.
opts=$(./ar-lib ./lib s foo.lib)
test x"$opts" = x

:
