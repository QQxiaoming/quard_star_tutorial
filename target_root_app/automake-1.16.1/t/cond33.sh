#!/bin/sh
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

# Check that conditionally-defined install directories are handled
# correctly.
# Report from Ralf Corsepius.

. test-init.sh

cat >>configure.ac <<'EOF'
AM_CONDITIONAL([INC], [test -z "$two"])
AC_OUTPUT
EOF

cat > Makefile.am <<'EOF'
if INC
include_foodir = $(includedir)/foo
include_foo_HEADERS = foo.h
else
bardir = $(bindir)
dist_bar_SCRIPTS = x.sh
endif

foo.h x.sh:
	:>$@

.PHONY: distdircheck
distdircheck: distdir
	test -f $(distdir)/foo.h
	test -f $(distdir)/x.sh
EOF

$ACLOCAL
$AUTOCONF
$AUTOMAKE

cwd=$(pwd) || fatal_ "cannot get current directory"
mkdir nowhere
chmod a-w nowhere

./configure --prefix="$cwd"/nowhere --bindir="$cwd"/bin \
            --includedir="$cwd"/inc
$MAKE installdirs
test ! -e bin
test -d inc/foo
test ! -e inc/foo/foo.h
rm -rf inc
$MAKE install
test ! -e bin
test -f inc/foo/foo.h
$MAKE distdircheck

rm -rf inc

./configure two=two --prefix="$cwd"/nowhere --bindir="$cwd"/bin \
                    --includedir="$cwd"/inc
$MAKE install
test ! -e inc
test -f bin/x.sh
rm -rf inc
$MAKE installdirs
test ! -e inc
test -d bin
$MAKE distdircheck

:
