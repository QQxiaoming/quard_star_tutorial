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

# Check against automake bug#9579: distcheck does not always detect
# incomplete uninstall as advertised.

. test-init.sh

cat >> configure.ac << 'END'
AC_OUTPUT
END

# NOTE: the use of 'dir' as the name of the data file installed by hand
# is deliberate, and enhances coverage -- see definition and comments of
# lib/am/distdir.am:$(am__distuninstallcheck_listfiles).

cat > Makefile.am << 'END'
dist_data_DATA = foo
EXTRA_DIST = dir
install-data-local:
	$(MKDIR_P) '$(DESTDIR)$(datadir)'
	cp '$(srcdir)/dir' '$(DESTDIR)$(datadir)/dir'
END

: > foo
: > dir

$ACLOCAL
$AUTOMAKE
$AUTOCONF

./configure --prefix="$(pwd)/inst"

# Sanity checks.
$MAKE install
find inst -type f
test -f inst/share/foo
test -f inst/share/dir
# We expect the uninstall target of our Makefile to be definitely broken.
$MAKE uninstall
test -f inst/share/dir
rm -rf inst

run_make -M -e FAIL distcheck
$FGREP 'ERROR: files left after uninstall:' output
grep '/share/dir *$' output

# A few trickier corner cases.

cat > Makefile.am << 'END'
EXTRA_DIST = dir
install-data-local:
install-data-local:
	$(MKDIR_P) '$(DESTDIR)$(prefix)/mu/share/info'
	cp '$(srcdir)/dir' '$(DESTDIR)$(prefix)/mu/share/info'
	$(MKDIR_P) '$(DESTDIR)$(infodir)/more'
	cp '$(srcdir)/dir' '$(DESTDIR)$(infodir)/more'
END

$AUTOMAKE
./config.status Makefile

# Sanity checks, again.
$MAKE install
find inst -type f
test -f inst/mu/share/info/dir
test -f inst/share/info/more/dir
# We expect the uninstall target of our Makefile to be definitely broken.
$MAKE uninstall
test -f inst/mu/share/info/dir
test -f inst/share/info/more/dir
rm -rf inst

run_make -M -e FAIL distcheck
$FGREP 'ERROR: files left after uninstall:' output
grep '/mu/share/info/dir *$' output
grep '/share/info/more/dir *$' output

:
