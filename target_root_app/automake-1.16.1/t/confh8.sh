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

# Test to make sure several config headers are allowed.
# See also sister "minimalistic" test 'confh7.sh'.

. test-init.sh

cat >> configure.ac << 'END'
AC_CONFIG_HEADERS([one.h two.h])
AC_OUTPUT
END

cat > Makefile.am << 'END'
.PHONY: test0 test1 test2
test0:
	@echo DIST_COMMON = $(DIST_COMMON)
	echo ' ' $(DIST_COMMON) ' ' | grep '[ /]one\.h\.in '
	echo ' ' $(DIST_COMMON) ' ' | grep '[ /]two\.h\.in '
	: Processed header files should not be distributed.
	if echo ' ' $(DIST_COMMON) ' ' | grep '\.h '; then \
	  exit 1; \
	else \
	  exit 0; \
	fi
test1: all
	test -f one.h
	test -f two.h
test2: distdir
	ls -l $(distdir)/*
	test -f $(distdir)/one.h.in
	test -f $(distdir)/two.h.in
	: Processed header files should not be distributed.
	test ! -r $(distdir)/one.h
	test ! -r $(distdir)/two.h
check-local: test0 test1 test2
END

: > one.h.in
: > two.h.in

$ACLOCAL
$AUTOCONF
$AUTOMAKE

./configure

$MAKE test0
$MAKE test1
$MAKE test2
$MAKE distcheck

:
