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

# Test per-target flags in vala support.

required="pkg-config valac gcc GNUmake"
. test-init.sh

mkdir src

cat >> configure.ac <<'END'
AC_PROG_CC
AM_PROG_VALAC([0.7.0])
PKG_CHECK_MODULES([GOBJECT], [gobject-2.0 >= 2.4])
AC_CONFIG_FILES([src/Makefile])
AC_OUTPUT
END

cat > Makefile.am <<'END'
SUBDIRS = src
END

cat > src/Makefile.am <<'END'
bin_PROGRAMS = foo bar
foo_CFLAGS = $(GOBJECT_CFLAGS)
foo_LDADD = $(GOBJECT_LIBS)
foo_SOURCES = xfoo.vala
bar_SOURCES = xbar.vala
bar_VALAFLAGS = -D BAR
bar_CFLAGS = $(GOBJECT_CFLAGS)
bar_LDADD = $(GOBJECT_LIBS)
END

cat > src/xfoo.vala <<'END'
int main ()
{
  stdout.printf ("foo\n");
  return 0;
}
END

cat > src/xbar.vala <<'END'
void main ()
{
#if BAR
  stdout.printf ("bar\n");
#else
  stdout.oops_an_invalid_method ();
#endif
}
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure
$MAKE

if ! cross_compiling; then
  ./src/foo
  ./src/bar
  test "$(./src/foo)" = foo
  test "$(./src/bar)" = bar
fi

# Test clean rules.

cp config.status config.sav

$MAKE clean
test -f src/xfoo.c
test -f src/xbar.c

$MAKE distclean
test -f src/xfoo.c
test -f src/xbar.c

# Re-create Makefile.
mv config.sav config.status
./config.status

$MAKE maintainer-clean
test ! -e src/xfoo.c
test ! -e src/xbar.c

:
