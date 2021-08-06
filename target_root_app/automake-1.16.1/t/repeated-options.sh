#! /bin/sh
# Copyright (C) 2010-2018 Free Software Foundation, Inc.
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

# Check that automake does not complain on repeated options, nor
# generate broken or incorrect makefiles.

required='cc bzip2'
. test-init.sh

cat >configure.ac <<END
AC_INIT([$me], [1.0])
AM_INIT_AUTOMAKE([foreign dist-bzip2 no-dist-gzip no-dist-gzip dist-bzip2])
AC_PROG_CC
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
END

cat > Makefile.am <<'END'
AUTOMAKE_OPTIONS = parallel-tests subdir-objects subdir-objects
AUTOMAKE_OPTIONS += dist-bzip2 parallel-tests
TESTS = foo.test
EXTRA_DIST = $(TESTS)
TESTS_ENVIRONMENT = EXEEXT='$(EXEEXT)'
bin_PROGRAMS = sub/foo
.PHONY: test-build
test-build:
	ls -l . sub
	test -f sub/foo.$(OBJEXT)
	test -f sub/foo$(EXEEXT)
END

mkdir sub

cat > foo.test <<'END'
#!/bin/sh
test -f sub/foo$EXEEXT && test -x sub/foo$EXEEXT
END
chmod a+x foo.test

cat > sub/foo.c <<'END'
int main (void)
{
  return 0;
}
END

cp "$am_scriptdir"/test-driver .

$ACLOCAL
$AUTOMAKE --foreign --foreign -Wall 2>stderr || { cat stderr >&2; exit 1; }
test -s stderr && { cat stderr >&2; exit 1; }
rm -f stderr
$AUTOCONF

./configure
$MAKE
$MAKE test-build
$MAKE check
ls -l
test -f foo.log
test -f test-suite.log
$MAKE clean
$MAKE distcheck
ls -l
test -f $me-1.0.tar.bz2
test ! -e $me-1.0.tar.gz

:
