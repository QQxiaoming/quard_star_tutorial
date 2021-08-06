#! /bin/sh
# Copyright (C) 2008-2018 Free Software Foundation, Inc.
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

# PR 516: Prefer generated manpages to distributed ones.

. test-init.sh

cat > Makefile.am << 'END'
dist_man_MANS = foo.1
installcheck-local:
	grep bar "$(mandir)/man1/foo.1"
END

cat >>configure.ac <<'END'
: ${foo=foo}
AC_SUBST([foo])
AC_CONFIG_FILES([foo.1])
AC_OUTPUT
END

cat > foo.1.in <<'END'
@foo@
END

$ACLOCAL
$AUTOMAKE
$AUTOCONF
./configure
run_make DISTCHECK_CONFIGURE_FLAGS='foo=bar' distcheck

:
