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

# Test EXTRA_*_DEPENDENCIES.  See 'extra-deps-lt.sh' for libtool variant.

required=cc
. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AM_PROG_AR
AC_PROG_RANLIB
AC_SUBST([deps], [bardep])
AM_CONDITIONAL([COND], [test -n "$cond"])
AC_OUTPUT
END

cat > Makefile.am << 'END'
noinst_LIBRARIES = libfoo.a
EXTRA_libfoo_a_DEPENDENCIES = libfoodep
libfoodep:
	@echo making $@
	@: > $@
CLEANFILES = libfoodep

bin_PROGRAMS = foo bar
EXTRA_foo_DEPENDENCIES = foodep
if COND
EXTRA_foo_DEPENDENCIES += foodep2
endif
bar_LDADD = libfoo.a
EXTRA_bar_DEPENDENCIES = $(deps)

EXTRA_DIST = foodep bardep

.PHONY: bar-has-been-updated
bar-has-been-updated:
	is_newest bar$(EXEEXT) libfoo.a
END

cat >libfoo.c <<'END'
int libfoo () { return 0; }
END

cat >foo.c <<'END'
int main () { return 0; }
END

cat >bar.c <<'END'
extern int libfoo ();
int main () { return libfoo (); }
END

$ACLOCAL
$AUTOMAKE --add-missing
$AUTOCONF

./configure cond=yes

# Hypotheses:
#  - EXTRA_*_DEPENDENCIES are honored.
#  - Conditionals and substitutions are honored.
#  - *_DEPENDENCIES are not overwritten by their EXTRA_* counterparts.

: >foodep
: >foodep2
: >bardep
run_make -O
grep 'making libfoodep' stdout

rm -f foodep
$MAKE && exit 1
: >foodep

rm -f foodep2
$MAKE && exit 1
: >foodep2

rm -f bardep
$MAKE && exit 1
: >bardep

$MAKE
$sleep
touch libfoo.a
$MAKE
$MAKE bar-has-been-updated

$MAKE distcheck

:
