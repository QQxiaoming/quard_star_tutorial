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

# Check that AUTOMAKE_OPTIONS support variable expansion.

. test-init.sh

# We want complete control over automake options.
AUTOMAKE=$am_original_AUTOMAKE

cat > configure.ac <<END
AC_INIT([$me], [1.0])
AM_INIT_AUTOMAKE([-Wall -Werror gnu])
AC_CONFIG_FILES([Makefile])
END

cat > Makefile.am <<'END'
# The following should expand to '-Wnone -Wno-error foreign -Wportability'.
AUTOMAKE_OPTIONS = $(foo) foreign
AUTOMAKE_OPTIONS += ${bar}
foo = $(foo1)
foo1 = ${foo2}
foo2 = -Wnone
foo2 += $(foo3)
foo3 = -Wno-error
bar = -Wportability
# This will give a warning with '-Wportability'.
zardoz :=
# This would give a warning with '-Woverride'.
install:
END

$ACLOCAL
AUTOMAKE_run
grep '^Makefile\.am:.*:=.*not portable' stderr
grep README stderr && exit 1
$EGREP '(install|override)' stderr && exit 1

:
