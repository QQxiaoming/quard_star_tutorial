#! /bin/sh
# Copyright (C) 2009-2018 Free Software Foundation, Inc.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

# Complain about configure substitutions in EXTRA_PRIMARY variables.
# Rationale: we need to know the literal values of, say, PROGRAMS.
# Thus, we allow substitutions in bin_PROGRAMS, but then EXTRA_PROGRAMS
# needs to list possible values of that substitution, literally.

. test-init.sh

cat >>configure.ac <<'END'
AC_PROG_CC
AC_SUBST([substed1])
AC_SUBST([substed2])
END

cat > Makefile.am <<'END'
bin_PROGRAMS = @substed1@
EXTRA_PROGRAMS = @substed2@
END

$ACLOCAL
AUTOMAKE_fails
grep 'bin_PROGRAMS.*contains configure substitution' stderr && exit 1
grep 'EXTRA_PROGRAMS.*contains configure substitution' stderr

exit 0
