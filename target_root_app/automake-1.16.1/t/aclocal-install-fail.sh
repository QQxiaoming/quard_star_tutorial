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

# Check that "aclocal --install" fails when it should.
# FIXME: this is a good candidate for a conversion to TAP.

am_create_testdir=empty
required=ro-dir
. test-init.sh

cat > configure.ac <<END
AC_INIT([$me], [1.0])
MY_MACRO
END

mkdir sys-acdir
cat > sys-acdir/my-defs.m4 <<END
AC_DEFUN([MY_MACRO], [:])
END

ACLOCAL="$ACLOCAL -Wnone --system-acdir=sys-acdir"

: > a-regular-file
mkdir unwritable-dir
chmod a-w unwritable-dir

$ACLOCAL -I a-regular-file --install 2>stderr \
  && { cat stderr >&2; exit 1; }
cat stderr >&2
$EGREP '(mkdir:|directory ).*a-regular-file' stderr
test ! -e aclocal.m4

$ACLOCAL --install -I unwritable-dir/sub 2>stderr \
  && { cat stderr >&2; exit 1; }
cat stderr >&2
$EGREP '(mkdir:|directory ).*unwritable-dir/sub' stderr
test ! -e aclocal.m4

$ACLOCAL -I unwritable-dir --install 2>stderr \
  && { cat stderr >&2; exit 1; }
cat stderr >&2
$EGREP '(cp:|copy ).*unwritable-dir' stderr
test ! -e aclocal.m4

# Sanity check.
mkdir m4
$ACLOCAL -I m4 --install && test -f aclocal.m4 \
  || fatal_ "aclocal failed also when expected to succeed"

:
