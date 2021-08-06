#! /bin/sh
# Copyright (C) 2003-2018 Free Software Foundation, Inc.
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

# Test to make sure that AC_CONFIG_LINKS using a variable source
# is not broken.

. test-init.sh

cat > Makefile.am << 'END'
SUBDIRS = sdir
.PHONY: test
test: distdir
	test ! -r $(distdir)/sdir/dest3
	test ! -r $(distdir)/sdir/dest2
	test ! -r $(distdir)/dest3
	test ! -r $(distdir)/dest2
	test -f $(distdir)/src2
## src3 cannot be distributed, Automake knows nothing about it.
	test ! -r $(distdir)/sdir/src3
	test ! -r $(distdir)/src3
END

: > src
: > src2
mkdir sdir
: > sdir/Makefile.am
: > sdir/src3

cat >>configure.ac << 'EOF'
AC_CONFIG_FILES([sdir/Makefile])
my_src_dir=sdir
my_dest=dest
AC_CONFIG_LINKS([sdir/dest2:src2 sdir/dest3:$my_src_dir/src3])
AC_CONFIG_LINKS([$my_dest:src])
# The following is a link whose source is itself a link.
AC_CONFIG_LINKS([dest4:sdir/dest2])
# Some package prefer to compute links.
cmplink='dest5:src';
AC_CONFIG_LINKS([$cmplink])
AC_OUTPUT
EOF

$ACLOCAL
$AUTOCONF
$AUTOMAKE

# $my_src_dir and $my_dest are variables local to configure, they should
# not appear in Makefile.
grep my_src_dir Makefile.in && exit 1
grep my_dest Makefile.in && exit 1

./configure
test -r sdir/dest2
test -r sdir/dest3
test -r dest
test -r dest4
test -r dest5
$MAKE test

$MAKE distclean
test ! -e sdir/dest2
test ! -e sdir/dest3
test -r dest  # Should still exist, Automake knows nothing about it.
test -r dest5 # Ditto.
rm -f dest dest5
test ! -e dest4

mkdir build
cd build
../configure
$MAKE test

:
