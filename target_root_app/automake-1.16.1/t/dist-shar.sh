#! /bin/sh
# Copyright (C) 2013-2018 Free Software Foundation, Inc.
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

# Check support for no-dist-gzip with dist-shar.

required='shar unshar'
. test-init.sh

errmsg='support for shar .*deprecated'

echo AUTOMAKE_OPTIONS = dist-shar > Makefile.am
$ACLOCAL
AUTOMAKE_fails -Wnone -Wobsolete
grep "^Makefile\\.am:1:.*$errmsg" stderr

cat > configure.ac <<END
AC_INIT([$me], [1.0])
AM_INIT_AUTOMAKE([no-dist-gzip dist-shar])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
END
: > Makefile.am

rm -rf autom4te*.cache
$ACLOCAL
AUTOMAKE_run -Wno-error
grep "^configure\\.ac:2:.*$errmsg" stderr

$AUTOCONF
./configure
$MAKE distcheck
test -f $distdir.shar.gz

:
