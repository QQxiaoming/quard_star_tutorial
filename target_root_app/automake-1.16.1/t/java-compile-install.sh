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

# Test on compilation and installation of Java class files.

required=javac
. test-init.sh

cat >>configure.ac <<'EOF'
AC_OUTPUT
EOF

cat > Makefile.am << 'END'
javadir = $(datadir)/java
java_JAVA = Foo.java
nobase_java_JAVA = Foo2.java
nobase_dist_java_JAVA = Bar.java
nodist_java_JAVA = Baz.java

# Java files are not distributed by default, so we distribute
# one "by hand" ...
EXTRA_DIST = Foo.java
# ... and make the other one generated.
Foo2.java:
	rm -f $@ $@-t
	echo 'class bClass {}' > $@-t
	chmod a-w $@-t && mv -f $@-t $@

# Explicitly declared as 'nodist_', so generate it.
Baz.java:
	rm -f $@ $@-t
	echo 'class Baz {}' > $@-t
	echo 'class Baz2 {}' >> $@-t
	chmod a-w $@-t && mv -f $@-t $@

test:
	ls -l $(srcdir) . ;: For debugging.
	test   -f $(srcdir)/Foo.java
	test   -f $(srcdir)/Bar.java
	test   -f Foo2.java
	test   -f Baz.java
	test   -f aClass.class
	test   -f bClass.class
	test   -f Zardoz.class
	test   -f Baz.class
	test   -f Baz2.class
	test ! -r Foo.class
	test ! -r Bar.class
	test   -f classjava.stamp

test-install:
	ls -l $(javadir) ;: For debugging.
	test   -f '$(javadir)/bClass.class'
	test   -f '$(javadir)/aClass.class'
	test   -f '$(javadir)/Zardoz.class'
	test   -f '$(javadir)/Baz.class'
	test   -f '$(javadir)/Baz2.class'
	test ! -r '$(javadir)/Foo.class'
	test ! -r '$(javadir)/Bar.class'
	if find $(prefix) | grep '\.stamp$$'; then exit 1; else :; fi

check-local: test
installcheck-local: test-install

.PHONY: test test-install

DISTCLEANFILES = Baz.java Foo2.java
END

echo 'class aClass {}' > Foo.java
echo 'class Zardoz {}' > Bar.java

$ACLOCAL
$AUTOCONF
$AUTOMAKE

./configure --prefix="$(pwd)"/_inst
$MAKE
$MAKE test
$MAKE install
$MAKE test-install
$MAKE distcheck

:
