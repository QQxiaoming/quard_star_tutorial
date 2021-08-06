#! /bin/sh
# Copyright (C) 2003-2018 Free Software Foundation, Inc.
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

# Make sure dependencies on aclocal.m4 are set correctly.
# Report from Jim Meyering.

required=cc
. test-init.sh

cat >>configure.ac <<EOF
AC_CONFIG_MACRO_DIR([m4])
AC_PROG_RANLIB
AM_PROG_AR
AC_PROG_CC
MY_MACRO
AC_CONFIG_FILES([lib/Makefile])
AC_OUTPUT
EOF

mkdir m4
cat >m4/mymacro.m4 <<EOF
AC_DEFUN([MY_MACRO], [])
EOF

mkdir lib
: > lib/foo.c
: > lib/bar.c
cat >lib/Makefile.am <<'EOF'
noinst_LIBRARIES = liberi.a
liberi_a_SOURCES = bar.c
liberi_a_LIBADD = $(LIBOBJS)
EOF

cat >Makefile.am <<'EOF'
SUBDIRS = lib
EXTRA_DIST = m4/mymacro.m4
check-foo: distdir
	test -f $(distdir)/lib/foo.c
	test -f $(distdir)/lib/bar.c

check-not-foo: distdir
	test ! -f $(distdir)/lib/foo.c
	test -f $(distdir)/lib/bar.c
EOF

$ACLOCAL
$AUTOCONF
$AUTOMAKE --add-missing
./configure
$MAKE check-not-foo

# Update one of the macros.  This should cause ./configure, Makefile.in,
# Makefile, lib/Makefile.in, and lib/Makefile to be updated.

cat >m4/mymacro.m4 <<'EOF'
AC_DEFUN([MY_MACRO], [AC_LIBOBJ([foo])])
EOF

using_gmake || $MAKE
$MAKE check-foo

:
