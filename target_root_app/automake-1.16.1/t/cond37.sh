#!/bin/sh
# Copyright (C) 2004-2018 Free Software Foundation, Inc.
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

# Check conditional local rules.
# Report from Simon Josefsson.

. test-init.sh

cat >>configure.ac <<'EOF'
AM_CONDITIONAL([CASE_A], [test -n "$case_A"])
AC_OUTPUT
EOF

cat > Makefile.am <<'EOF'
if CASE_A
check-local:
	@echo GrepMe1
else
install-data-local:
	@echo GrepMe2
endif
EOF

$ACLOCAL
$AUTOCONF
$AUTOMAKE

./configure
run_make -O check
grep GrepMe1 stdout && exit 1
run_make -O install
grep GrepMe2 stdout

./configure case_A=1
run_make -O check
grep GrepMe1 stdout
run_make -O install
grep GrepMe2 stdout && exit 1

:
