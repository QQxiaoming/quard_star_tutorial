#! /bin/sh
# Copyright (C) 2001-2018 Free Software Foundation, Inc.
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

# Make sure nobase_* works for libtool libraries and programs as well.
# This is just the libtool equivalent of 'nobase.sh', split up to allow
# greater exposure of that test.

required='cc libtoolize'
. test-init.sh

cat >> configure.ac <<'EOF'
AC_PROG_CC
AM_PROG_AR
AC_PROG_LIBTOOL
AC_OUTPUT
EOF

cat > Makefile.am << 'EOF'
fooexecdir = $(prefix)/foo
fooexec_LTLIBRARIES = sub/libbase.la
nobase_fooexec_LTLIBRARIES = sub/libnobase.la
fooexec_PROGRAMS = sub/base
nobase_fooexec_PROGRAMS = sub/nobase
sub_libbase_la_SOURCES = source2.c
sub_libnobase_la_SOURCES = source2.c
sub_base_SOURCES = source.c
sub_nobase_SOURCES = source.c

test-install-data: install-data
	test ! -f inst/foo/sub/libnobase.la
	test ! -f inst/foo/libbase.la

test-install-exec: install-exec
	test   -f inst/foo/sub/libnobase.la
	test ! -f inst/foo/libnobase.la
	test   -f inst/foo/libbase.la

.PHONY: test-install-exec test-install-data
EOF

mkdir sub

cat >source.c <<'EOF'
int main (int argc, char *argv[])
{
  return 0;
}
EOF
cp source.c source2.c

rm -f install-sh

libtoolize
$ACLOCAL
$AUTOCONF
$AUTOMAKE -a --copy
./configure --prefix "$(pwd)/inst" --program-prefix=p

$MAKE
$MAKE test-install-data
$MAKE test-install-exec
$MAKE uninstall

test $(find inst/foo -type f -print | wc -l) -eq 0

$MAKE install-strip

# Likewise, in a VPATH build.

$MAKE uninstall
$MAKE distclean
mkdir build
cd build
../configure --prefix "$(pwd)/inst" --program-prefix=p
$MAKE
$MAKE test-install-data
$MAKE test-install-exec
$MAKE uninstall
test $(find inst/foo -type f -print | wc -l) -eq 0

:
