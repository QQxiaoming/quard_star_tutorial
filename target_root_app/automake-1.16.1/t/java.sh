#! /bin/sh
# Copyright (C) 1998-2018 Free Software Foundation, Inc.
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

# Minimal test of Java functionality.

required=javac
. test-init.sh

cat >>configure.ac <<'EOF'
AC_OUTPUT
EOF

cat > Makefile.am << 'END'
javadir = $(datadir)/java
dist_java_JAVA = a.java b.java
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE

$EGREP '\.stamp|class' Makefile.in # For debugging.
grep '^all[-a-z]*:.*classjava\.stamp' Makefile.in
test $(grep -c '^all[-a-z]*:.*classjava\.stamp' Makefile.in) -eq 1

echo 'class a { }' > a.java
echo 'class b { }' > b.java

./configure
$MAKE distcheck

:
