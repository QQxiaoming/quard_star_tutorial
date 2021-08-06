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

# Test for bug in conditionals.  From Raja R Harinath.

. test-init.sh

cat >> configure.ac << 'END'
AM_CONDITIONAL([WRONG], [test x = y])
AC_OUTPUT
END

cat > Makefile.am << 'END'
if WRONG
this=
else
this=is_something_interesting
endif

.PHONY: test-this
test-this:
	test '$(this)' = is_something_interesting
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a
./configure
$MAKE test-this

:
