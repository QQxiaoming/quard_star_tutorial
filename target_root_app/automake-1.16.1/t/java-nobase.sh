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

# Test support of 'nobase_' with the 'JAVA' primary.

required=javac
. test-init.sh

cat >>configure.ac <<'END'
AC_OUTPUT
END

cat > Makefile.am << 'END'
javadir = $(datadir)/java
nobase_java_JAVA = sub/subsub/Foo.java
nobase_dist_java_JAVA = sub/Bar.java
nobase_nodist_java_JAVA = sub2/Baz.java

# Java files are not distributed by default.
EXTRA_DIST = sub/subsub/Foo.java

sub2/Baz.java:
	rm -f $@ $@-t
	test -d sub2 || mkdir sub2
	echo 'class Baz {}' > $@-t
	chmod a-w $@-t && mv -f $@-t $@

.PHONY: test-install
test-install:
	find $(prefix) ;: For debugging.
	test   -f $(javadir)/sub/subsub/Foo.class
	test   -f $(javadir)/sub/Bar.class
	test   -f $(javadir)/sub2/Baz.class
installcheck-local: test-install
DISTCLEANFILES = sub2/Baz.java
END

mkdir sub sub/subsub
echo 'class Foo {}' > sub/subsub/Foo.java
echo 'class Bar {}' > sub/Bar.java

$ACLOCAL
$AUTOCONF
$AUTOMAKE

./configure --prefix="$(pwd)"/_inst
$MAKE check
$MAKE install
$MAKE test-install
$MAKE distcheck

:
