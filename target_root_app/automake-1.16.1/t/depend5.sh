#! /bin/sh
# Copyright (C) 2008-2018 Free Software Foundation, Inc.
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

# Check that _AM_OUTPUT_DEPENDENCY_COMMANDS works with eval-style
# quoting in $CONFIG_FILES, done by newer Autoconf.

required=cc
. test-init.sh

cat >>configure.ac << END
AC_PROG_CC
AC_OUTPUT
END

cat > Makefile.am << END
bin_PROGRAMS = foo
foo_SOURCES = foo.c foo.h
END

cat >foo.c << END
#include "foo.h"
END
: >foo.h

$ACLOCAL
$AUTOMAKE
$AUTOCONF
./configure --enable-dependency-tracking
if test -d .deps; then
  depdir=.deps
elif test -d _deps; then
  depdir=_deps
else
  depdir=
fi

# For the fun of it, we should also cope with Makefile names
# that contain weird characters and newer.
# Pick the first name that the file system will accept.
for name in \
  'weird  name with $ `#() &! characters"' \
  'weird  name with $ `#()  characters"' \
  'weird  name with  characters'
do
  cp Makefile.in "$name.in" && break || :
done

for arg in Makefile \
  --file=Makefile \
  "--file=$name"
do
  rm -rf .deps _deps
  ./config.status "$arg" depfiles >stdout 2>stderr ||
    { cat stdout; cat stderr >&2; exit 1; }
  cat stdout
  cat stderr >&2
  grep '[Nn]o such file' stderr && exit 1

  if test -n "$depdir"; then
    test -d $depdir || exit 1
  fi
done

:
