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

# Test pre-processing substitutions '%reldir%' and '%canon_reldir%'
# with C compilation and subdir objects.

require=cc
. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AM_PROG_CC_C_O
AC_CONFIG_FILES([zot/Makefile])
AC_OUTPUT
END

mkdir foo
mkdir foo/bar
mkdir foo/foobar
mkdir zot

cat > Makefile.am << 'END'
AUTOMAKE_OPTIONS = subdir-objects
SUBDIRS = zot
bin_PROGRAMS =

include $(top_srcdir)/foo/local.mk
include $(srcdir)/foo/foobar/local.mk
include local.mk

check-local:
	is $(bin_PROGRAMS) == \
	  foo/mumble2$(EXEEXT) \
	  foo/bar/mumble$(EXEEXT) \
	  foo/foobar/mumble$(EXEEXT) \
	  mumble$(EXEEXT)
	test '$(mumble_SOURCES)' = one.c
	test '$(foo_mumble2_SOURCES)' = foo/one.c
	test '$(foo_bar_mumble_SOURCES)' = foo/bar/one.c
	test '$(foo_foobar_mumble_SOURCES)' = foo/foobar/one.c
	test -f mumble$(EXEEXT)
	test -f foo/mumble2$(EXEEXT)
	test -f foo/bar/mumble$(EXEEXT)
	test -f foo/foobar/mumble$(EXEEXT)
	test -f zot/mumble$(EXEEXT)
	: Test some of the object files too.
	test -f one.$(OBJEXT)
	test -f foo/foobar/one.$(OBJEXT)
	test -f zot/one.$(OBJEXT)
END

cat > zot/Makefile.am << 'END'
AUTOMAKE_OPTIONS = subdir-objects
bin_PROGRAMS =
include $(top_srcdir)/zot/local.mk

test:
	test '$(bin_PROGRAMS)' = mumble$(EXEEXT)
	test '$(mumble_SOURCES)' = one.c
check-local: test
END

cat > local.mk << 'END'
bin_PROGRAMS += %reldir%/mumble
%canon_reldir%_mumble_SOURCES = %reldir%/one.c
END

echo 'int main (void) { return 0; }' > one.c

sed 's/mumble/mumble2/' local.mk > foo/local.mk
cp local.mk foo/bar
cp local.mk foo/foobar
cp local.mk zot
echo "include %reldir%/bar/local.mk" >> foo/local.mk

cp one.c foo
cp one.c foo/bar
cp one.c foo/foobar
cp one.c zot

$ACLOCAL
$AUTOCONF
$AUTOMAKE
./configure

$MAKE
$MAKE check-local
if ! cross_compiling; then
  ./mumble
  ./foo/mumble2
  ./foo/bar/mumble
  ./foo/foobar/mumble
  ./zot/mumble
fi

(cd zot && $MAKE test)

# GNU install refuses to override a just-installed file; since we
# have plenty of 'mumble' dummy programs to install in the same
# location, such "overridden installations" are not a problem for
# us; so just force the use the 'install-sh' script.
ac_cv_path_install=$(pwd)/install-sh; export ac_cv_path_install
$MAKE distcheck

:
