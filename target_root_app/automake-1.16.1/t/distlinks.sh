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

# Check that distributed symlinks in the source tree will be expanded
# as regular files in $(distdir).

. test-init.sh

echo text > file

ln -s file lnk || skip_ "cannot create symlinks to files"

mkdir A
mkdir B
echo aaa > A/aaa
cd B
ln -s ../A/aaa bbb
cd ..

echo FooBarBaz > foo

ln -s foo  bar1
ln -s bar1 bar2
ln -s bar2 bar3

ln -s "$(pwd)/foo" quux

cat >> configure.ac << 'END'
AC_OUTPUT
END

echo "me = $me" > Makefile.am # For better failure messages.
cat >> Makefile.am << 'END'
EXTRA_DIST = lnk B/bbb bar1 bar2 bar3 quux
.PHONY: test
test: distdir
	ls -l $(distdir) $(distdir)/B
	fail() { echo "$(me): $$*" >&2; e=1; }; \
	e=0; \
	set file lnk A/aaa B/bbb foo quux foo bar1 foo bar2 foo bar3; \
	while test $$# -ge 2; do \
	  file=$$1; shift; link=$(distdir)/$$1; shift; \
	  test -f $$link || fail "$$link is not a regular file"; \
	  test ! -h $$link || fail "$$link is a symlink"; \
	  diff $$file $$link || fail "$$link differs from $$file"; \
	done; \
	exit $$e;
END

ls -l . A B

$ACLOCAL
$AUTOCONF
$AUTOMAKE

./configure
$MAKE test

:
