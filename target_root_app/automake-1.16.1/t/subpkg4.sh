#! /bin/sh
# Copyright (C) 2006-2018 Free Software Foundation, Inc.
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

# Test to ensure 'distdir' fixes the mode of files and directories
# copied into '$(distdir)', even with sub-packages.

. test-init.sh

cat >Makefile.am <<'END'
SUBDIRS = subpkg
test-distdir-is-readable:
	bad_dirs=`find $(distdir) -type d ! -perm -755 -print`; \
	if test -n "$$bad_dirs"; then \
	  echo "directories not permissive: $$bad_dirs" >&2; \
	  exit 1; \
	fi
	bad_files=`find $(distdir) ! -type d ! -perm -444 -print`; \
	if test -n "$$bad_files"; then \
	  echo "file not permissive: $$bad_files" >&2; \
	  exit 1; \
	fi
END

cat >>configure.ac <<'END'
AC_CONFIG_SUBDIRS([subpkg])
AC_OUTPUT
END

mkdir subpkg subpkg/dir
echo foobar >subpkg/foobar
echo baz > subpkg/dir/baz

chmod go-rwx subpkg/foobar subpkg/dir/baz subpkg/dir

cat >subpkg/Makefile.am <<'END'
EXTRA_DIST = foobar dir
END

cat >subpkg/configure.ac <<'END'
AC_INIT([subpkg], [1.0])
AM_INIT_AUTOMAKE
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
END

$ACLOCAL
$AUTOMAKE
$AUTOCONF

cd subpkg
$ACLOCAL
$AUTOMAKE
$AUTOCONF
cd ..

./configure
$MAKE distdir
$MAKE test-distdir-is-readable
