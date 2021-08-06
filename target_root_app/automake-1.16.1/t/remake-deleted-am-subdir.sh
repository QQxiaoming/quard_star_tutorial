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
# of the "deleted header problem".  This test does the check when the
# SUBDIRS variable is involved.

. test-init.sh

cat >> configure.ac <<'END'
AC_CONFIG_FILES([sub1/Makefile sub2/Makefile])
AC_OUTPUT
END

$ACLOCAL
$AUTOCONF

cat > Makefile.am <<'END'
include foo.am
SUBDIRS = sub1 sub2
END

echo '# this is foo.am' > foo.am

mkdir sub1 sub2

echo 'include $(srcdir)/bar.am' > sub1/Makefile.am
echo '# this is bar.am' > sub1/bar.am

echo 'include $(top_srcdir)/foo.am' > sub2/Makefile.am

$AUTOMAKE
# Sanity checks.
$FGREP 'this is foo.am' Makefile.in
$FGREP 'this is bar.am' sub1/Makefile.in
$FGREP 'this is foo.am' sub2/Makefile.in

./configure
$MAKE # Should be no-op.

$sleep
echo '# this is sub1/Makefile.am' > sub1/Makefile.am
rm -f sub1/bar.am
$MAKE all
# Sanity checks.
$FGREP 'this is bar' sub1/Makefile.in sub1/Makefile && exit 1
$FGREP 'this is sub1/Makefile.am' sub1/Makefile.in
$FGREP 'this is sub1/Makefile.am' sub1/Makefile

$sleep
for d in . sub2; do
  sed "s|.*include.*foo\.am.*|# this is $d/Makefile.am|" $d/Makefile.am > t
  mv -f t $d/Makefile.am
done
rm -f foo.am
$MAKE all
# Sanity checks.
$FGREP 'this is foo' sub*/Makefile* Makefile* && exit 1
for d in . sub1 sub2; do
  $FGREP "this is $d/Makefile.am" $d/Makefile.in
  $FGREP "this is $d/Makefile.am" $d/Makefile
done

:
