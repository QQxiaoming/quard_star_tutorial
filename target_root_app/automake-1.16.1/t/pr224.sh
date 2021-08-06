#! /bin/sh
# Copyright (C) 2002-2018 Free Software Foundation, Inc.
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

# Test for PR automake/224: $(top_srcdir) w.r.t. dependencies.
#
# == Description ==
# Dependency tracking data should be stored in in the relative path
# of the source file, as opposed the object file.
# I.e., for 'foo.c' in '$(top_srcdir)/bar', being built in
# '$(top_srcdir)/build', the location of the '.deps' directory
# should be '$(top_builddir)/bar/.deps'.

required=cc
. test-init.sh

mkdir foo

cat >foo/main.c <<'EOF'
int main (void)
{
  return 0;
}
EOF

cat >Makefile.am <<'EOF'
AUTOMAKE_OPTIONS = subdir-objects
bin_PROGRAMS = bar
bar_SOURCES = foo/main.c
EOF

cat >>configure.ac <<'EOF'
AC_PROG_CC
AC_OUTPUT
EOF

mkdir build

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

cd build
../configure

$MAKE
test -f foo/.deps/main.Po
$MAKE distclean
test ! -e foo/.deps/main.Po

:
