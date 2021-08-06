#! /bin/sh
# Copyright (C) 2011-2018 Free Software Foundation, Inc.
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

# Check that inclusion of '.am' fragments by automake does not suffer
# of the "deleted header problem".  Basic checks are done here.

. test-init.sh

echo AC_OUTPUT >> configure.ac

$ACLOCAL
$AUTOCONF

cat > Makefile.am <<'END'
include foo.am
include $(srcdir)/bar.am
include $(top_srcdir)/baz.am
END

echo '# this is foo' > foo.am
echo '# this is bar' > bar.am
echo '# this is baz' > baz.am

$AUTOMAKE
# Sanity checks.
$FGREP 'this is foo' Makefile.in
$FGREP 'this is bar' Makefile.in
$FGREP 'this is baz' Makefile.in

./configure
$MAKE # Should be no-op.

$sleep
sed '/^include foo\.am$/d' Makefile.am > t
mv -f t Makefile.am
rm -f foo.am
$MAKE Makefile
# Sanity checks.
$FGREP 'this is foo' Makefile.in Makefile && exit 1
$FGREP 'this is bar' Makefile.in
$FGREP 'this is bar' Makefile
$FGREP 'this is baz' Makefile.in
$FGREP 'this is baz' Makefile

$sleep
echo '# empty empty' > Makefile.am
rm -f bar.am baz.am
$MAKE Makefile
# Sanity checks.
$FGREP 'empty empty' Makefile.in
$FGREP 'empty empty' Makefile
grep 'this is ba[rz]' Makefile Makefile.in && exit 1

:
