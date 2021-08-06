#! /bin/sh
# Copyright (C) 2011-2018 Free Software Foundation, Inc.
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

# Files specified by AC_REQUIRE_AUX_FILE must exist, and if a Makefile
# is present in the build-aux directory, they correctly get automatically
# distributed.
# Related to automake bug#9651.

am_create_testdir=empty
. test-init.sh

cat > configure.ac <<END
AC_INIT([$me], [1.0])
AC_CONFIG_AUX_DIR([sub])
AM_INIT_AUTOMAKE
AC_REQUIRE_AUX_FILE([zardoz])
AC_CONFIG_FILES([Makefile sub/Makefile])
AC_OUTPUT
END

cat > Makefile.am <<'END'
SUBDIRS = sub
test: distdir
	ls -l $(distdir) $(distdir)/* ;: For debugging.
	test -f $(distdir)/sub/zardoz
.PHONY: test
check-local: test
END

mkdir sub
: > sub/Makefile.am

echo dummy > sub/zardoz

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a
test -f sub/install-sh
# Should work also without '--add-missing'.
$AUTOMAKE

./configure

$MAKE test
$MAKE distcheck

:
