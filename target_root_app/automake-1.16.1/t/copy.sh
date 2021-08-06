#! /bin/sh
# Copyright (C) 1999-2018 Free Software Foundation, Inc.
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

# Test to make sure '-c' works.  Report from Andris Pavenis.
# See also the much more in-depth test 'add-missing'.

. test-init.sh

# We'll have to cater to systems like MSYS/MinGW where there are no
# true symlinks ('ln -s' behaves like 'cp -p'); see automake bug#10441.

echo dummy > foo
if ln -s foo bar && test -h bar; then
  is_symlink () { test -h "$1"; }
  is_not_symlink () { test ! -h "$1"; }
else
  is_symlink () { return 0; } # Avoid spurious failures.
  is_not_symlink () { return 0; }
fi
rm -f foo bar

# First a simple test, where the auxdir is automatically determined
# by automake.

: > Makefile.am
rm -f install-sh

$ACLOCAL
$AUTOMAKE -c -a
ls -l # For debugging.

test -f install-sh
is_not_symlink install-sh

# Let's do a couple of more elaborated tests, this time with the auxdir
# explicitly defined in configure.ac.

mkdir sub
cd sub

cat > configure.ac <<END
AC_INIT([$me], [1.0])
AC_CONFIG_AUX_DIR([auxdir])
AM_INIT_AUTOMAKE
AC_PROG_CC
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
END

cat > Makefile.am <<END
bin_PROGRAMS = foo
END

$ACLOCAL

# 'automake -a' called without '-c' should create symlinks by default,
# even when there is already a non-symlinked required auxiliary file.

mkdir auxdir
echo FAKE-DEPCOMP > auxdir/depcomp
$AUTOMAKE -a
ls -l auxdir # For debugging.
test -f auxdir/install-sh
is_symlink auxdir/install-sh
test -f auxdir/depcomp
is_not_symlink auxdir/depcomp
test FAKE-DEPCOMP = "$(cat auxdir/depcomp)"

# 'automake -a -c' should not create symlinks, even when there are
# already symlinked required auxiliary files.

rm -rf auxdir
mkdir auxdir
cd auxdir
ln -s "$am_scriptdir/missing" "$am_scriptdir/install-sh" .
cd ..

$AUTOMAKE -a -c
ls -l auxdir # For debugging.
test -f auxdir/install-sh
is_symlink auxdir/install-sh
test -f auxdir/missing
is_symlink auxdir/missing
test -f auxdir/depcomp
is_not_symlink auxdir/depcomp
diff "$am_scriptdir"/depcomp auxdir/depcomp

:
