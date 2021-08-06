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

# PR 507: Check the filename-length-max=99 option
# in conjunction with AC_CONFIG_SUBDIRS.

. test-init.sh

# The name is so that 99 is exactly hit (including final \0).
subdirname='cnfsubdir'

cat >>configure.ac <<END
AC_CONFIG_SUBDIRS([$subdirname])
AC_OUTPUT
END

cat >Makefile.am <<END
AUTOMAKE_OPTIONS = filename-length-max=99
SUBDIRS = $subdirname
END

mkdir $subdirname

cat >> $subdirname/configure.ac <<EOF
AC_INIT([$subdirname], [1.0])
AM_INIT_AUTOMAKE
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
EOF

cat >$subdirname/Makefile.am <<'END'
AUTOMAKE_OPTIONS = filename-length-max=99
EXTRA_DIST = 12345678
END

(cd $subdirname || exit 1
for i in 1 2 3 4 5 6 7 8; do
  mkdir -p 12345678 && cd 12345678 && touch x || exit 1
done) || skip_ "failed to create deep directory hierarchy"

# AIX 5.3 'cp -R' is too buggy for 'make dist'.
cp -R $subdirname t \
  || skip_ "'cp -R' failed to copy deep directory hierarchy"

cd $subdirname
$ACLOCAL
$AUTOCONF
$AUTOMAKE
cd ..

$ACLOCAL
$AUTOCONF
$AUTOMAKE

./configure
$MAKE distcheck

(cd $subdirname || exit 1
for i in 1 2 3 4 5 6 7 8 9; do
  mkdir -p 12345678 && cd 12345678 && touch x || exit 1
done) || skip_ "failed to create deeper directory hierarchy"

run_make -E -e FAIL dist
grep 'filenames are too long' stderr
test 1 -eq $(grep -c 12345678 stderr)

:
