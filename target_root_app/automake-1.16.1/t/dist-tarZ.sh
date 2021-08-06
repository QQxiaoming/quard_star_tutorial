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

# Check support for no-dist-gzip with dist-tarZ.

. test-init.sh

# On Cygwin, as of 9/2/2012, 'compress' is provided by sharutils
# and is just a dummy script that is not able to actually compress
# (it can only decompress).  So, check that the 'compress' program
# is actually able to compress input.
# Note that, at least on GNU/Linux, 'compress' does (and is
# documented to) exit with status 2 if the output is larger than
# the input after (attempted) compression; so we need to pass it
# an input that it can actually reduce in size when compressing.
for x in 1 2 3 4 5 6 7 8; do
  echo aaaaaaaaaaaaaaaaaaaaa
done | compress -c >/dev/null \
  || skip_ "cannot find a working 'compress' program"

errmsg=".*legacy .*'compress' .*deprecated"

echo AUTOMAKE_OPTIONS = dist-tarZ > Makefile.am
$ACLOCAL
AUTOMAKE_fails -Wnone -Wobsolete
grep "^Makefile\\.am:1:.*$errmsg" stderr

cat > configure.ac <<END
AC_INIT([$me], [1.0])
AM_INIT_AUTOMAKE([no-dist-gzip dist-tarZ])
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
test -f dist-tarz-1.0.tar.Z

:
