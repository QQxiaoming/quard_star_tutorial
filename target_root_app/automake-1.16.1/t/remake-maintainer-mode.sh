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

# Check that AM_MAINTAINER_MODE disable some rebuild rules,
# but not all.
# Report from Ralf Corsepius.

. test-init.sh

if using_gmake; then
  remake="$MAKE"
else
  remake="$MAKE Makefile"
fi

cat >>configure.ac <<'EOF'
AM_MAINTAINER_MODE
m4_include([foo.m4])
if test ! -f rebuild_ok; then
  ACLOCAL=false
  AUTOMAKE=false
  AUTOCONF=false
fi
AC_OUTPUT
EOF

: > foo.m4
: > Makefile.am

$ACLOCAL
$AUTOCONF
$AUTOMAKE --add-missing
./configure
$MAKE

# Make sure the rules to rebuild configure/Makefile.in are not
# triggered by default.  ($MAKE will fail if they are, because the
# tools are set to false.)
$sleep
touch aclocal.m4 Makefile.am configure.ac foo.m4
$remake

# Make sure the rebuild rule for Makefile is triggered.
$sleep
echo '# GrEpMe' >>Makefile.in
$remake
grep GrEpMe Makefile

# Make sure the rebuild rule for config.status is triggered.
$sleep
grep 'AUTOCONF.*=.*false' Makefile
: > rebuild_ok
./configure --no-create
$remake
grep 'AUTOCONF.*=.*false' Makefile && exit 1

# Make sure rebuild rules do work if --enable-maintainer-mode is given.
./configure --enable-maintainer-mode
$sleep
echo 'AC_SUBST([YIPPY_YIPPY_YEAH])' >foo.m4
$remake
grep YIPPY_YIPPY_YEAH Makefile

# Try the distribution, for completeness.
$MAKE distcheck

:
