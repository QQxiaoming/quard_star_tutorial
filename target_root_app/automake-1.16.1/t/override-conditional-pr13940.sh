#! /bin/sh
# Copyright (C) 2013-2018 Free Software Foundation, Inc.
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

# Expose automake bug#1394: automake erroneously think that a .PHONY
# target's rule is overridden only because we declare dependencies
# to such targets twice: one in an Automake conditional, and once
# unconditionally.

. test-init.sh

cat >> configure.ac << 'END'
AM_CONDITIONAL([FOO], [true])
AC_OUTPUT
END

cat > Makefile.am << 'END'
# The conditional here is important: Automake only appears to warn
# when one of the all-local rules is inside a conditional and the
# other is not.
if FOO
all-local: bar
bar: ; @echo bar
endif
all-local: baz
baz: ; @echo baz
END

$ACLOCAL
$AUTOMAKE -a

:
