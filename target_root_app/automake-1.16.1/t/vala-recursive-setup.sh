#! /bin/sh
# Copyright (C) 1996-2018 Free Software Foundation, Inc.
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

# Test to make sure compiling Vala code really works with recursive make.

required="pkg-config valac gcc GNUmake"
. test-init.sh

mkdir src

cat >> 'configure.ac' << 'END'
AC_PROG_CC
AM_PROG_VALAC([0.7.0])
PKG_CHECK_MODULES([GOBJECT], [gobject-2.0 >= 2.4])
AC_CONFIG_FILES([src/Makefile])
AC_OUTPUT
END

cat > 'Makefile.am' <<'END'
SUBDIRS = src
END

cat > 'src/Makefile.am' <<'END'
bin_PROGRAMS = zardoz
zardoz_VALAFLAGS = -H zardoz.h
zardoz_CFLAGS = $(GOBJECT_CFLAGS)
zardoz_LDADD = $(GOBJECT_LIBS)
zardoz_SOURCES = zardoz.vala
END

cat > 'src/zardoz.vala' <<'END'
using GLib;

public class Zardoz {
  public static void main () {
    stdout.printf ("Zardoz!\n");
  }
}
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure
$MAKE

# Test rebuild rules.

rm -f src/zardoz.h
$MAKE -C src zardoz.h
test -f src/zardoz.h
rm -f src/zardoz.c
$MAKE -C src
test -f src/zardoz.c

echo am--error > src/zardoz.h
echo am--error > src/zardoz.c
$sleep
touch src/zardoz.vala
$MAKE
grep 'am--error' src/zardoz.[ch] && exit 1

# Check the distribution.

$MAKE distcheck
$MAKE distclean

# Tru a VPATH setup.

mkdir build
cd build
../configure
$MAKE
$MAKE distcheck

# Test rebuild rules from builddir.

rm -f ../src/zardoz.h
$MAKE -C src ../../src/zardoz.h
test -f ../src/zardoz.h

rm -f ../src/zardoz.c
$MAKE
grep 'Zardoz!' ../src/zardoz.c

$sleep
sed 's/Zardoz!/FooBar!/' ../src/zardoz.vala > t
mv -f t ../src/zardoz.vala
$MAKE
grep 'FooBar!' ../src/zardoz.c
grep 'Zardoz!' ../src/zardoz.c && exit 1

:
