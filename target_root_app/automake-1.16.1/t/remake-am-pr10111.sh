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

# Check that our remake rules doesn't give spurious successes in
# some corner case situations where they should actually fail.
# See automake bug#10111.
# To be clear, we are speaking about *very* corner-case situations here,
# and the fact that the remake rules might get confused in them is not a
# big deal in practice (in fact, this test *currently fails*).  Still,
# keeping the limitation exposed is a good idea anyway.

. test-init.sh

cat >> configure.ac <<'END'
AC_OUTPUT
END

: > foobar.am

cat > Makefile.am <<'END'
include $(srcdir)/foobar.am
$(srcdir)/foobar.am:
## Creative quoting is to avoid spurious matches in the grepping
## of Makefile.in, later.
	echo "mu =" foobar "was here =" > $@
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE

./configure

# OK, so the developer wants to interactively try out how the
# "distributed form" of his package behaves.
$MAKE distdir
cd $distdir
# He's interested in trying out a VPATH build.
mkdir build
cd build
../configure
# He wants to verify that the rules he's written to rebuild a file
# included by configure.ac works also in VPATH builds.
rm -f ../foobar.am
$MAKE
grep '= foobar was here =' ../Makefile.in
$MAKE distcheck

:
