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

# Check for _DEPENDENCIES definition with conditional _LDADD.
# Report from Elena A. Vengerova.

. test-init.sh

cat >>configure.ac <<'EOF'
AM_CONDITIONAL([TWO], [test -n "$two"])
AC_OUTPUT
EOF

cat > Makefile.am <<'EOF'
OBJEXT=z
CC=false
AUTOMAKE_OPTIONS=no-dependencies

bin_PROGRAMS = test1 test2

if TWO
  test1_LDADD = two.$(OBJEXT)
  test2_LDADD = two.$(OBJEXT)
  test2_DEPENDENCIES = $(test2_LDADD) somethingelse.a
else !TWO
  test1_LDADD = one.$(OBJEXT)
  test2_LDADD = three.$(OBJEXT)
endif !TWO

test1_DEPENDENCIES = $(test1_LDADD) somethingelse.a

.PHONY: dep-test1 dep-test2
dep-test1:
	echo BEG: $(test1_DEPENDENCIES) :END
dep-test2:
	echo BEG: $(test2_DEPENDENCIES) :END
EOF

$ACLOCAL
$AUTOCONF
$AUTOMAKE
./configure

run_make -O dep-test1
$FGREP 'BEG: one.z somethingelse.a :END' stdout

run_make -O dep-test2
$FGREP 'BEG: three.z :END' stdout

./configure two=2

run_make -O dep-test1
$FGREP 'BEG: two.z somethingelse.a :END' stdout

run_make -O dep-test2
$FGREP 'BEG: two.z somethingelse.a :END' stdout

:
