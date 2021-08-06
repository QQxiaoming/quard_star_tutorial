#! /bin/sh
# Copyright (C) 2004-2018 Free Software Foundation, Inc.
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

# Check the tar options diagnostics.

. test-init.sh

cat > configure.ac <<END
AC_INIT([$me], [1.0])
AM_INIT_AUTOMAKE([tar-pax tar-v7])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
END

: > Makefile.am

$ACLOCAL
AUTOMAKE_fails
grep "^configure\.ac:2:.*mutually exclusive" stderr > tar-err
cat tar-err
test 1 -eq $(wc -l < tar-err)
grep "'tar-pax'" tar-err
grep "'tar-v7'"  tar-err

rm -rf autom4te.cache

cat > configure.ac <<END
AC_INIT([$me], [1.0])
AM_INIT_AUTOMAKE
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
END

echo 'AUTOMAKE_OPTIONS = tar-pax' > Makefile.am

AUTOMAKE_fails
grep "^Makefile\.am:1:.*'tar-pax'.*AM_INIT_AUTOMAKE" stderr

:
