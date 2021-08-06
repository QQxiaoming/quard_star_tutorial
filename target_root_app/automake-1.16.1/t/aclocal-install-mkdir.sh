#! /bin/sh
# Copyright (C) 2012-2018 Free Software Foundation, Inc.
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

# Check that "aclocal --install" creates the local m4 directory if
# necessary.
# FIXME: this is a good candidate for a conversion to TAP.

am_create_testdir=empty
. test-init.sh

cat > configure.ac <<END
AC_INIT([$me], [1.0])
MY_MACRO
END

mkdir sys-acdir
cat > sys-acdir/my-defs.m4 <<END
AC_DEFUN([MY_MACRO], [:])
END

ACLOCAL="$ACLOCAL --system-acdir=sys-acdir"

$ACLOCAL -I foo --install
test -f foo/my-defs.m4

$ACLOCAL --install -I "$(pwd)/bar"
test -f bar/my-defs.m4

$ACLOCAL --install -I baz/sub/sub2
test -f baz/sub/sub2/my-defs.m4

# What should happen:
#  * zardoz1 should be created, and required m4 files copied into there.
#  * zardoz2 shouldn't be preferred to quux, even if the former exists
#    while the latter does not.
mkdir zardoz2
$ACLOCAL --install -I zardoz1 -I zardoz2
test -d zardoz1
grep MY_MACRO zardoz1/my-defs.m4
ls zardoz2 | grep . && exit 1

# Directories in ACLOCAL_PATH should never be created if they don't
# exist.
ACLOCAL_PATH="$(pwd)/none:$(pwd)/none2" $ACLOCAL --install && exit 1
test ! -e none
test ! -e none2
ACLOCAL_PATH="$(pwd)/none:$(pwd)/none2" $ACLOCAL --install -I x
test -f x/my-defs.m4
test ! -e none
test ! -e none2

# It's better if aclocal doesn't create the first include dir on failure.
$ACLOCAL --install -I none -I none2 && exit 1
test ! -e none
test ! -e none2

:
