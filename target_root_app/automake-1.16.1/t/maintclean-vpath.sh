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

# Make sure distclean and maintainer-clean erase the right files.
# This test is for VPATH builds; see sister test 'maintclean.sh'
# for in-tree builds.

. test-init.sh

cat >> configure.ac << 'END'
AC_CONFIG_FILES([bar sub/Makefile])
AC_OUTPUT
END

cat > Makefile.am <<'END'
SUBDIRS = sub
noinst_DATA = foo.c
foo.c:
	touch foo.c
MAINTAINERCLEANFILES = foo.c
END

mkdir sub
cat > sub/Makefile.am <<'END'
noinst_SCRIPTS = zap
zap: zap.sh
	cp $(srcdir)/zap.sh $@ && chmod a+x $@
MAINTAINERCLEANFILES = zap
END

: > bar.in
: > sub/zap.sh

$ACLOCAL
$AUTOCONF
$AUTOMAKE

# Users can disable autom4te.cache.
if test -d autom4te.cache; then
  test_cache='test -d ../autom4te.cache'
else
  test_cache=:
fi

mkdir build

chmod a-w . sub

cd build

../configure
test -f bar

$MAKE
test -f foo.c
test -f sub/zap
$test_cache

$MAKE distclean
test ! -e bar
test ! -e Makefile
test ! -e sub/Makefile
test ! -e config.status
test -f foo.c
test -f sub/zap
test -f ../sub/zap.sh
$test_cache

../configure
test -f bar

$MAKE foo.c
test -f foo.c
cd sub
$MAKE zap
test -f zap
cd ..

chmod u+w ..

$MAKE maintainer-clean
test -f ../sub/zap.sh
test ! -e bar
test ! -e foo.c
test ! -e sub/zap
test ! -e Makefile
test ! -e sub/Makefile
test ! -e config.status
test ! -e ../autom4te.cache

:
