#! /bin/sh
# Copyright (C) 2004-2018 Free Software Foundation, Inc.
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

# Test to make sure that if an auxfile (here depcomp) is required
# by a subdir Makefile.am, it is distributed by that Makefile.am.

required=cc
. test-init.sh

cat >> configure.ac << 'END'
AC_CONFIG_FILES([subdir/Makefile])
AC_PROG_CC
AC_PROG_FGREP
AC_OUTPUT
END

cat > Makefile.am << 'END'
SUBDIRS = subdir
test-distdir: distdir
	test -f $(distdir)/depcomp
.PHONY: test-distdir
check-local: test-distdir
END

rm -f depcomp
mkdir subdir

cat > subdir/Makefile.am << 'END'
.PHONY: test-distcom
test-distcom:
	echo ' ' $(DIST_COMMON) ' ' | $(FGREP) ' $(top_srcdir)/depcomp '
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE
test ! -e depcomp

cat >> subdir/Makefile.am << 'END'
bin_PROGRAMS = foo
.PHONY: test-distcom
test-distcom:
	echo ' ' $(DIST_COMMON) ' ' | $(FGREP) ' $(top_srcdir)/depcomp '
check-local: test-distcom
END

cat > subdir/foo.c <<'END'
int main (void)
{
  return 0;
}
END

$AUTOMAKE -a subdir/Makefile
test -f depcomp

./configure
(cd subdir && $MAKE test-distcom)
$MAKE test-distdir

$MAKE distcheck

:
