#! /bin/sh
# Copyright (C) 1999-2018 Free Software Foundation, Inc.
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

# Test to make sure config.h works in a subdir.
# Report from Alexandre Oliva.

. test-init.sh

cat >> configure.ac << 'END'
AC_CONFIG_HEADERS([include/config.h])
AC_OUTPUT
END

cat > Makefile.am << 'END'
.PHONY: test1 test2
test1:
	@echo DIST_COMMON = $(DIST_COMMON)
	echo ' ' $(DIST_COMMON) ' ' | grep '[ /]acconfig\.h '
test2: distdir
	ls -l $(distdir)/*
	test -f $(distdir)/acconfig.h
check-local: test1 test2
END

mkdir include
: > include/config.h.in
: > acconfig.h

# The test used to fail if 'include/Makefile.am' was created (!)
: > include/Makefile.am

$ACLOCAL
$AUTOCONF
$AUTOHEADER
$AUTOMAKE

./configure

$MAKE test1
$MAKE test2
$MAKE distcheck

# Make sure re-running automake in a different way generates same
# Makefile.in.
mv Makefile.in Makefile.sav
$ACLOCAL
$AUTOMAKE Makefile
diff Makefile.sav Makefile.in

:
