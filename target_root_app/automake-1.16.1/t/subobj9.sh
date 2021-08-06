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

# Test for PR 312.
#
# == Report ==
# When using non-recursive make to build a libtoolize-library from
# sources in a subdirectory, 'make distcheck' fails because of incomplete
# cleanup. "make clean" tries to remove '*.o' and '.../<file>.lo' but
# forgets '.../<file>.o'.

required='c++ libtoolize'
. test-init.sh

cat > configure.ac << END
AC_INIT([$me], [1.0])
AM_INIT_AUTOMAKE([subdir-objects])
AC_PROG_CXX
AM_PROG_AR
AM_PROG_LIBTOOL
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
END

cat > Makefile.am << 'END'
noinst_LTLIBRARIES = libfoo.la
libfoo_la_SOURCES = src/foo.cc .//src/bar.cc  # The './/' is meant.
.PHONY: print
print:
	@echo BEG1: "$(LTCXXCOMPILE)" :1END
	@echo BEG2: "$(CXXLINK)" :2END
END

mkdir src
cat > src/foo.cc << 'END'
int doit2 (void);
int doit (void)
{
   return doit2();
}
END

cat > src/bar.cc << 'END'
int doit2 (void)
{
   return 23;
}
END


libtoolize --force
$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure

# Ensure './libtool --help' will use the right tool versions.
export AUTOCONF AUTOMAKE

# Opportunistically check that --tag=CXX is used when supported.
if ./libtool --help | grep tag=TAG; then
  run_make -O print
  grep 'BEG1: .*--tag=CXX.*--mode=compile.* :1END' stdout
  grep 'BEG2: .*--tag=CXX.*--mode=link.* :2END' stdout
fi

$MAKE
run_make -M distcheck
# GNU Make used to complain that the Makefile contained two rules
# for 'src/.dirstamp' and './/src/.dirstamp'.
grep 'overriding commands' output && exit 1

:
