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

# Test definition of automake-generated private Makefile variable
# '$(am__java_sources)'.

. test-init.sh

cat >> configure.ac << 'END'
AC_OUTPUT
END

cat > Makefile.am << 'END'
foodir = $(prefix)

foo_JAVA = a.java
dist_foo_JAVA = b.java
nodist_foo_JAVA = c.java
nobase_foo_JAVA = d.java
nobase_dist_foo_JAVA = e.java
nobase_nodist_foo_JAVA = f.java

.PHONY: debug
debug:
	@echo 'am__java_sources: "$(am__java_sources)"'
got:
	@lst='$(am__java_sources)'; \
         for f in $$lst; do echo $$f; done | sort > $@
END

$ACLOCAL
$AUTOMAKE
$AUTOCONF

cat > exp << 'END'
a.java
b.java
c.java
d.java
e.java
f.java
END

./configure
$MAKE debug
$MAKE got

cat got
cat exp
diff exp got

:
