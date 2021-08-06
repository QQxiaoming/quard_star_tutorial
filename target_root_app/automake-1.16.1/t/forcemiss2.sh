#! /bin/sh
# Copyright (C) 2000-2018 Free Software Foundation, Inc.
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

# Test to make sure that 'automake --force-missing' works, even with
# symlinked files.

. test-init.sh

rm -f install-sh

echo zot > foo
cp foo foo2

ln -s foo2 install-sh || skip_ "cannot create symlinks to files"

: > Makefile.am

$ACLOCAL
ls -l install-sh
$AUTOMAKE --add-missing --force-missing
ls -l install-sh

test -f install-sh
grep zot install-sh && exit 1
diff foo foo2

:
