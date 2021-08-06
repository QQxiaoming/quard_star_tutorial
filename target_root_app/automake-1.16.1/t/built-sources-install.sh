#! /bin/sh
# Copyright (C) 2002-2018 Free Software Foundation, Inc.
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

# Make sure 'install:' honors $(BUILT_SOURCES).
# PR/359.

. test-init.sh

cat >> configure.ac << 'END'
AC_CONFIG_FILES([dir/Makefile])
AC_OUTPUT
END

mkdir dir

cat > Makefile.am << 'END'
BUILT_SOURCES = built1
SUBDIRS = dir
built1:
	echo ok > $@
CLEANFILES = built1
install-data-hook:
	$(MKDIR_P) $(DESTDIR)$(prefix)/dir2
	cp built1 $(DESTDIR)$(prefix)/built1
	cp dir/built2 $(DESTDIR)$(prefix)/dir2/built3
uninstall-hook:
	rm -f $(DESTDIR)$(prefix)/built1
	rm -f $(DESTDIR)$(prefix)/dir2/built3
	rmdir $(DESTDIR)$(prefix)/dir2
installcheck-local:
	test -f $(prefix)/built1
	test -f $(prefix)/dir2/built3
END

cat > dir/Makefile.am << 'END'
BUILT_SOURCES = built2
built2:
## The next line ensures that command1.inc has been built before
## recurring into the subdir.
	cp ../built1 $@
CLEANFILES = built2
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE
./configure --prefix "$(pwd)/inst"

# Now make sure these two files are rebuilt during make install.
$MAKE install
test -f built1
test -f dir/built2
$MAKE installcheck
$MAKE distcheck

:
