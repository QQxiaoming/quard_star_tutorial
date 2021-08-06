#! /bin/sh
# Copyright (C) 2001-2018 Free Software Foundation, Inc.
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

# Test for PR 203.
# See also automake bug#11030.
#
# == Original Report for PR/203 ==
# Some standard targets are missing '-local' hooks.  For instance,
# installdirs is missing this.  Ideally this would be an automatic
# feature of any exported target.

. test-init.sh

echo AC_OUTPUT >> configure.ac

cat > Makefile.am << 'END'
foodir = $(datadir)/$(distdir)
installdirs-local:
	$(MKDIR_P) $(DESTDIR)$(foodir)
install-data-hook: installdirs-local
END

$ACLOCAL
$AUTOMAKE

test $(grep -c installdirs-local Makefile.in) -eq 4

cwd=$(pwd) || fatal_ "getting current working directory"

$AUTOCONF
./configure --prefix="$cwd/inst"

$MAKE installdirs
test -d inst/share/$me-1.0
rm -rf inst

$MAKE install
test -d inst/share/$me-1.0
rm -rf inst

./configure --prefix=/foo

$MAKE installdirs DESTDIR="$cwd/dest"
test -d dest/foo/share/$me-1.0
rm -rf dest

$MAKE install DESTDIR="$cwd/dest"
test -d dest/foo/share/$me-1.0
rm -rf dest

:
