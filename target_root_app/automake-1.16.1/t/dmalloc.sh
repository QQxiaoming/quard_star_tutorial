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

# Simple checks on the 'AM_WITH_DMALLOC' macro.

. test-init.sh

cat >> configure.ac << 'END'
AC_CONFIG_HEADERS([config.h])
AM_WITH_DMALLOC
AC_SUBST([LDFLAGS])
AC_OUTPUT
END

cat > config.h.in <<'END'
#undef WITH_DMALLOC
END

cat > Makefile.am <<'END'
check-with-dmalloc:
## The AM_WITH_DMALLOC assumes that the user is a developer, so that
## he is expected to install and make available the 'dmalloc' library
## by his own.  So we just check that proper linker and libs flags get
## added.
	echo ' ' $(LDFLAGS) ' ' | grep ' -g '
	echo ' ' $(LIBS) ' ' | grep ' -ldmalloc '
END

$ACLOCAL
$AUTOMAKE
$AUTOCONF

./configure
cat config.h # For debugging.
grep '^ */\* *# *undef  *WITH_DMALLOC *\*/ *$' config.h

./configure --without-dmalloc
cat config.h # For debugging.
grep '^ */\* *# *undef  *WITH_DMALLOC *\*/ *$' config.h

./configure --with-dmalloc
cat config.h # For debugging.
grep '^# *define  *WITH_DMALLOC  *1 *$' config.h
$MAKE check-with-dmalloc

:
