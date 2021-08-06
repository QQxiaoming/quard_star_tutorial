#! /bin/sh
# Copyright (C) 2013-2018 Free Software Foundation, Inc.
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

# Tests error messages when '%reldir%' and '%canon_reldir%' substitutions
# (and their shorthands '%D%' and '%C%') are involved.

. test-init.sh

cat >> configure.ac <<'END'
AC_PROG_CC
AC_PROG_RANLIB
AM_PROG_AR
END

: > ar-lib

mkdir sub sub/sub2

cat > Makefile.am <<'END'
%canon_reldir%_x1_SOURCES = bar.c
include sub/local.mk
END

cat > sub/local.mk <<'END'
AUTOMAKE_OPTIONS = -Wno-extra-portability
include %D%/sub2/more.mk
noinst_LIBRARIES = %reldir%-one.a %D%-two.a
%C%_x2_SOURCES = foo.c
END

cat > sub/sub2/more.mk <<'END'
%C%_UNDEFINED +=
END

$ACLOCAL
AUTOMAKE_fails

cat > expected << 'END'
sub/sub2/more.mk:1: sub_sub2_UNDEFINED must be set with '=' before using '+='
Makefile.am:2: 'sub/local.mk' included from here
sub/local.mk:2: 'sub/sub2/more.mk' included from here
sub/local.mk:3: 'sub-one.a' is not a standard library name
sub/local.mk:3: did you mean 'libsub-one.a'?
Makefile.am:2: 'sub/local.mk' included from here
sub/local.mk:3: 'sub-two.a' is not a standard library name
sub/local.mk:3: did you mean 'libsub-two.a'?
Makefile.am:2: 'sub/local.mk' included from here
sub/local.mk:4: variable 'sub_x2_SOURCES' is defined but no program or
sub/local.mk:4: library has 'sub_x2' as canonical name (possible typo)
Makefile.am:2: 'sub/local.mk' included from here
Makefile.am:1: variable 'x1_SOURCES' is defined but no program or
Makefile.am:1: library has 'x1' as canonical name (possible typo)
END

# We need to break these substitutions into multiple sed invocations
# to avoid spuriously triggering the 'sc_tests_logs_duplicate_prefixes'
# maintainer check.
sed -e '/warnings are treated as errors/d' stderr > t1
sed -e 's/: warning:/:/' t1 > t2
sed -e 's/: error:/:/' t2 > t3
sed -e 's/  */ /g' t3 > obtained

diff expected obtained

:
