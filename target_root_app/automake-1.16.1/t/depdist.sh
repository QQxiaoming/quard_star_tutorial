#! /bin/sh
# Copyright (C) 2001-2018 Free Software Foundation, Inc.
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

# Make sure depcomp is found for the distribution.
# From Eric Magnien.

required=cc
. test-init.sh

rm -f configure.ac

mkdir config
for i in *; do
   if test "$i" != "config"; then
      mv -f "$i" config
   fi
done

cat > configure.ac << END
AC_INIT([$me], [1.0])
AC_CONFIG_AUX_DIR([config])
AM_INIT_AUTOMAKE
AC_PROG_CC
AC_CONFIG_FILES([subdir/Makefile Makefile])
AC_OUTPUT
END

cat > Makefile.am << 'END'
SUBDIRS = subdir
check-local: test1 test2
.PHONY: test1 test2
test1:
	echo ' ' $(DIST_COMMON) ' ' | grep '[ /]config/depcomp '
test2: distdir
	ls -l $(distdir)/*
	test -f $(distdir)/config/depcomp
END

mkdir subdir
cat > subdir/foo.c << 'END'
int main(void)
{
  return 0;
}
END

cat > subdir/Makefile.am << 'END'
bin_PROGRAMS = foo
foo_SOURCES = foo.c
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE --add-missing

./configure

$MAKE check
$MAKE distcheck

:
