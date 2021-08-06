#!/bin/sh
# Copyright (C) 2009-2018 Free Software Foundation, Inc.
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

# Silent rules: use of pre-defined variables $(AM_V_GEN) and $(AM_V_at).
# Incidentally, also check that silent rules are disabled by default.

. test-init.sh

echo AC_OUTPUT >> configure.ac

cat > Makefile.am <<'EOF'
all-local: foo

## And here's how you should do it in your own code:
foo: foo.in
	$(AM_V_GEN)cp $(srcdir)/foo.in $@
	$(AM_V_at)echo more >> $@

EXTRA_DIST = foo.in
CLEANFILES = foo
EOF

: >foo.in

$ACLOCAL
$AUTOMAKE --add-missing
$AUTOCONF

# Silent rules are disabled by default, since we haven't called
# "AM_SILENT_RULES([yes])" explicitly.
./configure
run_make -O
grep 'GEN ' stdout && exit 1
grep 'cp ' stdout
grep 'echo ' stdout

$MAKE clean
run_make -O V=1
grep 'GEN ' stdout && exit 1
grep 'cp ' stdout
grep 'echo ' stdout

$MAKE clean
run_make -O V=0
grep 'GEN .*foo' stdout
grep 'cp ' stdout && exit 1
grep 'echo ' stdout && exit 1

$MAKE distclean

./configure --enable-silent-rules
run_make -O
grep 'GEN .*foo' stdout
grep 'cp ' stdout && exit 1
grep 'echo ' stdout && exit 1

$MAKE clean
run_make -O V=0
grep 'GEN .*foo' stdout
grep 'cp ' stdout && exit 1
grep 'echo ' stdout && exit 1

$MAKE clean
run_make -O V=1
grep 'GEN ' stdout && exit 1
grep 'cp ' stdout
grep 'echo ' stdout

:
