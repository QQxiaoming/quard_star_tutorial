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

# Try a DIST_SUBDIRS subpackage with no SUBDIRS.
# Report from Gary V. Vaughan.

. test-init.sh

mkdir m4

cat >m4/foo.m4 <<'EOF'
AC_DEFUN([FOO],[
  AC_OUTPUT
])
EOF

cat >>configure.ac <<'END'
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SUBDIRS([sub])
AC_OUTPUT
END

cat >Makefile.am <<'EOF'
DIST_SUBDIRS = sub
dist-hook:
	test -f $(distdir)/sub/script.in
EOF

mkdir sub

cat >sub/configure.ac <<'EOF'
AC_INIT([sub], [2.3])
AM_INIT_AUTOMAKE
AC_CONFIG_MACRO_DIR([../m4])
AC_CONFIG_FILES([Makefile])
AC_CONFIG_FILES([script])
FOO
EOF

: >sub/script.in
: >sub/Makefile.am

$ACLOCAL
$AUTOCONF
$AUTOMAKE

cd sub
$ACLOCAL
$FGREP 'm4_include([../m4/foo.m4])' aclocal.m4
$AUTOCONF
$AUTOMAKE -Wno-override
cd ..

./configure
$MAKE distcheck

:
