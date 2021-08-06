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

# Make sure that we can enable or disable warnings on a per-file basis.

. test-init.sh

cat >>configure.ac <<END
AC_CONFIG_FILES([sub/Makefile])
AC_OUTPUT
END

mkdir sub

# These two Makefile contain the same errors, but have different
# warnings disabled.

cat >Makefile.am <<END
AUTOMAKE_OPTIONS = -Wno-obsolete
INCLUDES = -Ifoo
foo_SOURCES = unused
SUBDIRS = sub
END

cat >sub/Makefile.am <<END
AUTOMAKE_OPTIONS = -Wno-syntax
INCLUDES = -Ifoo
foo_SOURCES = unused
END

$ACLOCAL
AUTOMAKE_fails
# The expected diagnostic is
#   Makefile.am:3: warning: variable 'foo_SOURCES' is defined but no program or
#                           library has 'foo' as canonical name (possible typo)
#   sub/Makefile.am:2: warning: 'INCLUDES' is the old name for 'AM_CPPFLAGS'
grep '^Makefile.am:.*foo_SOURCES' stderr
grep '^sub/Makefile.am:.*INCLUDES' stderr
grep '^sub/Makefile.am:.*foo_SOURCES' stderr && exit 1
grep '^Makefile.am:.*INCLUDES' stderr && exit 1
# Only three lines of warnings.
test $(grep -v 'warnings are treated as errors' stderr | wc -l) -eq 3

# On fast machines the autom4te.cache created during the above run of
# $AUTOMAKE is likely to have the same time stamp as the configure.ac
# created below; thus causing traces for the old configure.ac to be
# used.  We could do '$sleep', but it's faster to erase the
# directory.  (Erase autom4te*.cache, not autom4te.cache, because some
# bogus installations of Autoconf use a versioned cache).
rm -rf autom4te*.cache

# If we add a global -Wnone, all warnings should disappear.
cat >configure.ac <<END
AC_INIT([warnopts], [1.0])
AM_INIT_AUTOMAKE([-Wnone])
AC_CONFIG_FILES([Makefile sub/Makefile])
AC_OUTPUT
END
$ACLOCAL
$AUTOMAKE

:
