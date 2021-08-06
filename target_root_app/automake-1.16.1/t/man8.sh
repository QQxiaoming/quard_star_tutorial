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

# Check for a bug in distcheck w.r.t. generated manpages.

. test-init.sh

# Avoid a spurious failure due to a known FreeBSD make incompatibility.
useless_vpath_rebuild \
  && skip_ "VPATH useless rebuild detected (see bug#7884)"

cat > Makefile.am << 'END'
dist_man_MANS = foo.1
foo.1:
## This 'rm' command will fail if $(srcdir) is unwritable.
	rm -f $(srcdir)/$@
	: > $(srcdir)/$@
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
test -f ../foo.1
$MAKE distdir
test -f $me-1.0/foo.1
$MAKE distcheck

cd ..
rm -f foo.1
./configure

$MAKE
test -f foo.1
$MAKE distdir
test -f $me-1.0/foo.1
$MAKE distcheck

:
