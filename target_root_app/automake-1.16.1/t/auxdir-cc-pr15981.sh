#! /bin/sh
# Copyright (C) 2014-2018 Free Software Foundation, Inc.
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

# Test automake bug#15981: automake 1.14 may use $ac_aux_dir
# before it is defined, leading to error messages such as:
# "/bin/sh: /home/david/missing: No such file or directory"

required=cc
. test-init.sh

cat > configure.ac <<END
AC_INIT([$me], [0.0])
AC_USE_SYSTEM_EXTENSIONS
AM_INIT_AUTOMAKE([1.11 foreign])
AC_CONFIG_FILES([Makefile])
AC_SUBST([MISSING])
AC_OUTPUT
test -n "\${MISSING}"
END

echo 'int main (void) { return 0; }' > foo.c

cat > Makefile.am <<'END'
bin_PROGRAMS = foo

# Without quotes around '--help' and with an empty $(MISSING), make might
# strip the trailing '--', call Bash's 'help' builtin, and have this test
# succeed spuriously.  Yes, that has happened in practice :-(
test:
	$(MISSING) '--help'
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE

./configure 2>stderr || { cat stderr >&2; exit 1; }
cat stderr >&2
$FGREP "missing" stderr && exit 1

$MAKE all
$MAKE test

:
