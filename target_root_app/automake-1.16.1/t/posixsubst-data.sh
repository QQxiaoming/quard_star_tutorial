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

# Test that POSIX variable expansion '$(var:str=rpl)' works
# in when used with the DATA primary.

. test-init.sh

cat >> configure.ac << 'END'
AC_OUTPUT
END

cat > Makefile.am << 'END'
foodir = $(prefix)

t1 = foo1 foo2
t2 = barx
t3 = baz.y

bar.data bazzardoz:
	: > $@
CLEANFILES = bar.data bazzardoz

# Also try an empty match suffix, to ensure that the ':=' in there is
# not confused by the parser with an unportable assignment operator.
dist_foo_DATA = $(t1:=.txt)
foo_DATA = $(t2:x=.data)
nodist_foo_DATA = $(t3:.y=zardoz)

check-local: test
.PHONY: test
test: distdir
	ls -l $(distdir)
	test ! -r $(distdir)/foo.data
	test ! -r $(distdir)/bazzardoz

installcheck-local:
	ls -l $(prefix)
	test -f $(prefix)/foo1.txt
	test -f $(prefix)/foo2.txt
	test -f $(prefix)/bar.data
	test -f $(prefix)/bazzardoz
END

: > foo1.txt
: > foo2.txt

$ACLOCAL
$AUTOCONF
$AUTOMAKE

cwd=$(pwd) || fatal_ "getting current working directory"
./configure --prefix="$cwd/_inst"
$MAKE install
test -f bar.data
test -f bazzardoz
$MAKE test
$MAKE installcheck
$MAKE distcheck

:
