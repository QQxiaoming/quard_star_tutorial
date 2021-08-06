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

# Test EXTRA_*_DEPENDENCIES, libtool version; see 'extra-deps.sh' for
# non-libtool variant.

required='cc libtoolize'
. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AM_PROG_AR
AC_PROG_LIBTOOL
AC_SUBST([deps], [bardep])
AC_OUTPUT
END

cat > Makefile.am << 'END'
noinst_LTLIBRARIES = libfoo.la
EXTRA_libfoo_la_DEPENDENCIES = libfoodep
libfoodep:
	@echo making $@
	@: > $@
CLEANFILES = libfoodep

bin_PROGRAMS = bar
bar_LDADD = libfoo.la
EXTRA_bar_DEPENDENCIES = $(deps)

EXTRA_DIST = bardep

.PHONY: bar-has-been-updated
bar-has-been-updated:
	is_newest bar$(EXEEXT) libfoo.la
END

cat >libfoo.c <<'END'
int libfoo () { return 0; }
END

cat >bar.c <<'END'
extern int libfoo ();
int main () { return libfoo (); }
END

libtoolize
$ACLOCAL
$AUTOMAKE --add-missing
$AUTOCONF

./configure

# Hypothesis: EXTRA_*_DEPENDENCIES are honored.

: >foodep
: >foodep2
: >bardep
run_make -O
grep 'making libfoodep' stdout

rm -f bardep
$MAKE && exit 1
: >bardep

$MAKE
$sleep
touch libfoo.la
$MAKE
$MAKE bar-has-been-updated

$MAKE distcheck

:
