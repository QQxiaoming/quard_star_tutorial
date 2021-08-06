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

# Subpackages that want to use m4 macros from their superpackages,
# with AC_CONFIG_MACRO_DIRS.

. test-init.sh

cat > configure.ac <<'END'
AC_INIT([super], [1.0])
AM_INIT_AUTOMAKE
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_SUBDIRS([pkg])
AX_BAR
AX_FOO
END

mkdir m4

cat > m4/foo.m4 <<'EOF'
AC_DEFUN([AX_FOO], [
  AC_CONFIG_FILES([Makefile])
  AC_OUTPUT
])
EOF

cat > m4/bar.m4 <<'EOF'
AC_DEFUN([AX_BAR], [AC_SUBST([WHOAMI], [SuperPkg])])
EOF

cat > Makefile.am << 'END'
test-whoami:
	test '$(WHOAMI)' = SuperPkg
check-local: test-whoami
END

mkdir pkg

cat > pkg/configure.ac <<'END'
AC_INIT([super], [1.0])
AM_INIT_AUTOMAKE
AC_CONFIG_MACRO_DIRS([macros ../m4])
AX_BAR
AX_FOO
END

mkdir pkg/macros
cat > pkg/macros/zardoz.m4 << 'END'
AC_DEFUN([AX_BAR], [AC_SUBST([WHOAMI], [sub-pkg])])
END

cat > pkg/Makefile.am << 'END'
test-whoami:
	test '$(WHOAMI)' = sub-pkg
check-local: test-whomai
END

AUTOMAKE=$AUTOMAKE ACLOCAL=$ACLOCAL AUTOCONF=$AUTOCONF $AUTORECONF -vi

$FGREP 'm4_include([m4/foo.m4])' aclocal.m4
$FGREP 'm4_include([m4/bar.m4])' aclocal.m4
$FGREP 'm4_include([../m4/foo.m4])' pkg/aclocal.m4
$FGREP 'm4_include([macros/zardoz.m4])' pkg/aclocal.m4

./configure

$MAKE test-whoami
(cd pkg && $MAKE test-whoami) || exit 1

$MAKE distcheck

:
