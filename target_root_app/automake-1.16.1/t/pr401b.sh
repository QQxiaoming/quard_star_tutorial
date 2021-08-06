#! /bin/sh
# Copyright (C) 2005-2018 Free Software Foundation, Inc.
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

# Check support for AC_CONFIG_LIBOBJ_DIR vs LTLIBOBJS.
# (pr401.sh and pr401c.sh do the same for LIBOBJS and ALLOCA)

required='cc libtoolize'
. test-init.sh

mkdir lib src

cat >lib/feep.c <<'EOF'
const char *feep (void)
{
  return "feep";
}
EOF

cat >src/main.c <<'EOF'
#include <stdio.h>

extern const char *feep (void);

int main (void)
{
  puts (feep ());
  return 0;
}
EOF

cat >>configure.ac << 'EOF'
## These lines are activated for later tests
#: AC_CONFIG_LIBOBJ_DIR([lib])
AC_PROG_CC
AC_LIBOBJ([feep])
AC_LIBSOURCE([feep.c])
AM_PROG_AR
AC_PROG_LIBTOOL
AC_CONFIG_FILES([lib/Makefile src/Makefile])
AM_CONDITIONAL([CROSS_COMPILING], [test $cross_compiling = yes])
AC_OUTPUT
EOF

## -------------------------------------------- ##
## First a test of traditional LTLIBOBJS usage. ##
## -------------------------------------------- ##

cat >Makefile.am <<'EOF'
SUBDIRS = lib src
EOF

cat >lib/Makefile.am <<'EOF'
noinst_LTLIBRARIES = libfeep.la
libfeep_la_SOURCES =
libfeep_la_LIBADD = $(LTLIBOBJS)
EOF

cat >src/Makefile.am <<'EOF'
check_PROGRAMS = main
main_LDADD = ../lib/libfeep.la

if !CROSS_COMPILING
TESTS = main
endif
EOF

cp "$am_scriptdir/ar-lib" . || fatal_ "fetching auxiliary script 'ar-lib'"

libtoolize
$ACLOCAL
$AUTOCONF
$AUTOMAKE -a
./configure
$MAKE distcheck

## ----------------------------------------- ##
## Traditional LTLIBOBJS with LIBOBJDIR set. ##
## ----------------------------------------- ##

# Invocation of AC_CONFIG_LIBOBJ_DIR may be necessary for reasons
# unrelated to Automake or Makefile.am layout.

sed 's/#: //' configure.ac >configure.tmp
mv -f configure.tmp configure.ac

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a
./configure
test ! -e lib/lib
$MAKE distcheck

## -------------------------------------------- ##
## Error message with usage in wrong directory. ##
## -------------------------------------------- ##

mv -f src/Makefile.am src/t
sed 's/LDADD = .*/LDADD = @LTLIBOBJS@/' src/t > src/Makefile.am
AUTOMAKE_fails
grep 'cannot be used outside.*lib' stderr
mv -f src/t src/Makefile.am

## ---------------------------------------------- ##
## Test using LTLIBOBJS from a sibling directory. ##
## ---------------------------------------------- ##

sed 's/lib\/Makefile //' configure.ac >configure.tmp
mv -f configure.tmp configure.ac

cat >Makefile.am <<'EOF'
SUBDIRS = src
EOF

cat > src/Makefile.am <<'EOF'
AUTOMAKE_OPTIONS = subdir-objects

noinst_LTLIBRARIES = libfeep.la
libfeep_la_SOURCES =
libfeep_la_LIBADD = $(LTLIBOBJS)

check_PROGRAMS = main
main_LDADD = libfeep.la

if !CROSS_COMPILING
TESTS = main
endif
EOF

$ACLOCAL
$AUTOCONF
$AUTOMAKE --add-missing
./configure
test ! -e src/lib
test ! -e 'src/$(top_builddir)'
$MAKE
$MAKE check
$MAKE distclean


## ------------------------------------------- ##
## Test using LTLIBOBJS from parent directory. ##
## ------------------------------------------- ##

sed 's/^.*src\/Makefile.*$//' configure.ac >configure.tmp
mv -f configure.tmp configure.ac

cat >Makefile.am <<'EOF'
AUTOMAKE_OPTIONS = subdir-objects

noinst_LTLIBRARIES = lib/libfeep.la
lib_libfeep_la_SOURCES =
lib_libfeep_la_LIBADD = $(LTLIBOBJS)

check_PROGRAMS = src/main
src_main_SOURCES = src/main.c
src_main_LDADD = lib/libfeep.la

if !CROSS_COMPILING
TESTS = src/main
endif

check-local:
	test -f src/main.$(OBJEXT)
	test -f lib/feep.lo
	test ! -f src/$(DEPDIR)/feep.Po
EOF

$ACLOCAL
$AUTOCONF
$AUTOMAKE
./configure
$MAKE distcheck

:
