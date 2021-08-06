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

# Check that 'acconfig.h' is *not* automatically distributed when
# placed in a subdirectory.
# Related to automake bug#7819.

. test-init.sh

cat >> configure.ac <<END
AC_CONFIG_FILES([sub/Makefile])
AC_OUTPUT
END

cat > Makefile.am <<'END'
SUBDIRS = sub
sub/acconfig.h:
	echo target $@ should not be built >&2; exit 1
check-local: distdir
	ls -l $(distdir)/sub
	test ! -f $(distdir)/sub/acconfig.h
END

mkdir sub

cat > sub/Makefile.am <<'END'
acconfig.h:
	echo target $@ should not be built >&2; exit 1
check-local:
	echo $(DISTFILES) | grep 'acconfig\.h' && exit 1; :
	echo $(DIST_COMMON) | grep 'acconfig\.h' && exit 1; :
END

: > sub/acconfig.h

$ACLOCAL
$AUTOMAKE
$AUTOCONF

./configure
$MAKE check

:
