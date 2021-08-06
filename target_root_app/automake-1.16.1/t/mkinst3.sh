#! /bin/sh
# Copyright (C) 2005-2018 Free Software Foundation, Inc.
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

# Test mkinstalldirs with spaces in directory names.

am_create_testdir=empty
. test-init.sh

cwd=$(pwd) || fatal_ "getting current working directory"

# Make sure the directory we will create can be created...
mkdir '~a b' && mkdir '~a b/-x  y' \
  || skip_ "directory names with spaces and metacharacters not accepted"
rm -rf '~a b'

get_shell_script mkinstalldirs

# Test mkinstalldirs with the installed mkdir.

./mkinstalldirs '~a b/-x  y'
test -d '~a b/-x  y'
rm -rf '~a b'

# Trick mkinstalldirs into thinking mkdir does not support -p.

mkdir bin
cat >bin/mkdir <<'EOF'
#!/bin/sh
for arg
do
  case $arg in
  -p) exit 1;;
  esac
done
PATH=$AM_PATH
export PATH
exec mkdir "$@"
EOF

chmod +x bin/mkdir

AM_PATH=$PATH; export AM_PATH
PATH=$cwd/bin$PATH_SEPARATOR$PATH; export PATH

# Test mkinstalldirs without mkdir -p.

./mkinstalldirs '~a b/-x  y'
test -d '~a b/-x  y'
rm -rf '~a b'

./mkinstalldirs "$cwd///~a b//-x  y"
test -d "$cwd/~a b/-x  y"
rm -rf '~a b'

:
