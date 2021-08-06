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

# Test for regressions in computation of names of .Po files for
# automatic dependency tracking.
# Keep this in sync with sister test 'depcomp8b.sh', which checks the
# same thing for libtool objects.

required=cc
. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AC_OUTPUT
END

cat > Makefile.am << 'END'
bin_PROGRAMS = zardoz
zardoz_SOURCES = foo.c sub/bar.c
END

mkdir sub
cat > foo.c << 'END'
int main (void)
{
  extern int bar (void);
  return bar ();
}
END
cat > sub/bar.c << 'END'
int bar (void)
{
  return 0;
}
END

$ACLOCAL
# FIXME: stop disabling the warnings in the 'unsupported' category
# FIXME: once the 'subdir-objects' option has been mandatory.
$AUTOMAKE -a -Wno-unsupported
grep '\.P' Makefile.in # For debugging.
grep '\./\$(DEPDIR)/foo\.Po' Makefile.in
grep '\./\$(DEPDIR)/bar\.Po' Makefile.in
grep '/\./\$(DEPDIR)' Makefile.in && exit 1

$AUTOCONF
# Don't reject slower dependency extractors, for better coverage.
./configure --enable-dependency-tracking
$MAKE
cross_compiling || ./zardoz
DISTCHECK_CONFIGURE_FLAGS='--enable-dependency-tracking' $MAKE distcheck

# Try again with subdir-objects option.

echo AUTOMAKE_OPTIONS = subdir-objects >> Makefile.am

$AUTOMAKE
grep '\.P' Makefile.in # For debugging.
grep '\./\$(DEPDIR)/foo\.Po' Makefile.in
grep '[^a-zA-Z0-9_/]sub/\$(DEPDIR)/bar\.Po' Makefile.in
$EGREP '/(\.|sub)/\$\(DEPDIR\)' Makefile.in && exit 1

$AUTOCONF
# Don't reject slower dependency extractors, for better coverage.
./configure --enable-dependency-tracking
$MAKE
cross_compiling || ./zardoz
DISTCHECK_CONFIGURE_FLAGS='--enable-dependency-tracking' $MAKE distcheck

:
