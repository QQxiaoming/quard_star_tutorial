#! /bin/sh
# Copyright (C) 2010-2018 Free Software Foundation, Inc.
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

# Check for a bug in maintainer-clean w.r.t. generated manpages.

. test-init.sh

cat > Makefile.am << 'END'
dist_man_MANS = $(srcdir)/foo.1 bar.1
$(srcdir)/foo.1 bar.1:
	: > $@
MAINTAINERCLEANFILES = $(dist_man_MANS)
END

cat >> configure.ac <<'END'
AC_OUTPUT
END

$ACLOCAL
$AUTOMAKE
$AUTOCONF

mkdir build
cd build
../configure

$MAKE
test -f bar.1
test -f ../foo.1

$MAKE maintainer-clean
test ! -e bar.1
test ! -e ../foo.1

cd ..
./configure

$MAKE
test -f bar.1
test -f foo.1

$MAKE maintainer-clean
test ! -e bar.1
test ! -e foo.1

:
