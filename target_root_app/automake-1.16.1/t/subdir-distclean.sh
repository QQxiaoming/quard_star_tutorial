#! /bin/sh
# Copyright (C) 2012-2018 Free Software Foundation, Inc.
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

# Check that "./configure && make && make distclean" is actually a
# no-op, even when conditional SUBDIRS are involved.

. test-init.sh

cat >> configure.ac << 'END'
AC_CONFIG_FILES([sub1/Makefile sub2/Makefile sub1/subsub/Makefile])
AM_CONDITIONAL([COND], [false])
AC_SUBST([extra_subdirs], [''])
AC_OUTPUT
END

mkdir sub1 sub2 sub1/subsub

cat > Makefile.am << 'END'
SUBDIRS = sub1
if COND
SUBDIRS += sub2
endif
END

cat > sub1/Makefile.am << 'END'
all-local:
	: > run
CLEANFILES = run
SUBDIRS = @extra_subdirs@
DIST_SUBDIRS = subsub
END

cat > sub2/Makefile.am << 'END'
all-local:
	@echo "Should not run in `pwd`!"
	exit 1
DISTCLEANFILES = oops
END
cp sub2/Makefile.am sub1/subsub/Makefile.am

$ACLOCAL
$AUTOCONF
$AUTOMAKE -c --add-missing

./configure

test -f sub1/Makefile
test -f sub2/Makefile
test -f sub1/subsub/Makefile

$MAKE
test -f sub1/run
touch sub2/oops sub1/subsub/oops

$MAKE distclean
test ! -e sub1/run
test ! -e sub2/oops
test ! -e sub1/subsub/oops
test ! -e sub1/Makefile
test ! -e sub2/Makefile
test ! -e sub1/subsub/Makefile

mkdir build
cd build

../configure

$MAKE

test -f sub1/Makefile
test -f sub2/Makefile
test -f sub1/subsub/Makefile

test -f sub1/run
touch sub2/oops sub1/subsub/oops

$MAKE maintainer-clean
test ! -e sub1/run
test ! -e sub2/oops
test ! -e sub1/subsub/oops
test ! -e sub1/Makefile
test ! -e sub2/Makefile
test ! -e sub1/subsub/Makefile

cd ..

./configure
$MAKE distclean

:
