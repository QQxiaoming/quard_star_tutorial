#! /bin/sh
# Copyright (C) 2012-2018 Free Software Foundation, Inc.
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

# Vala sources and C sources in the same program.  Functional test.

required='valac cc pkg-config GNUmake'
. test-init.sh

cat >> configure.ac <<'END'
AC_PROG_CC
AM_PROG_VALAC([0.7.3])
PKG_CHECK_MODULES([GOBJECT], [gobject-2.0 >= 2.4])
AC_OUTPUT
END

cat > Makefile.am <<'END'
bin_PROGRAMS = zardoz mu baz
AM_CFLAGS = $(GOBJECT_CFLAGS)
LDADD = $(GOBJECT_LIBS)
zardoz_SOURCES = foo.vala bar.c
mu_SOURCES = 1.vala 2.c
mu_VALAFLAGS = --main=run
mu_CFLAGS = -DHAVE_MU $(AM_CFLAGS)
baz_SOURCES = baz.c
END

if ! cross_compiling; then
  unindent >> Makefile.am <<'END'
    check-local:
	./zardoz
	./mu
	./zardoz | grep "foo is alive"
	./mu | grep "Howdy, World!"
END
fi

cat > foo.vala <<'END'
int main ()
{
  stdout.printf ("foo is alive\n");
  return 0;
}
END

echo 'extern int i = 0;' > bar.c

cat > 1.vala <<'END'
int run ()
{
  stdout.printf ("Howdy, World!\n");
  return 0;
}
END

cat > 2.c <<'END'
#ifdef HAVE_MU
int all_is_ok = 1;
#else
#error "HAVE_MU no defined"
chocke me
#endif
END

# For automake bug#11229.
cat > baz.c <<'END'
int main (void)
{
  return 0;
}
END

$ACLOCAL
$AUTOMAKE -a
$AUTOCONF

./configure

$MAKE all
ls -l # For debugging.
$MAKE check

have_generated_files ()
{
  test -f mu_vala.stamp
  test -f zardoz_vala.stamp
  test -f foo.c
  test -f 1.c
}

# Our vala-related rules must create stamp files and intermediate
# C files.
have_generated_files

# Remake rules are not uselessly triggered.
$MAKE -q
$MAKE -n | $FGREP vala.stamp && exit 1

# Check the distribution.
$MAKE distcheck

# Stamp files and intermediate C files should *not* be removed
# by "make clean".
$MAKE clean
have_generated_files

# But stamp files should be removed by "maintainer-clean" (the
# behaviour w.r.t. intermediate C files is still unclear, and
# better left undefined for the moment).
$MAKE maintainer-clean
ls *vala*.stamp | grep . && exit 1

:
