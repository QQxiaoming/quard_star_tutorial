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

# Check that the testsuite driver (either with or without the
# serial-tests option enabled) exports the 'srcdir' value in the
# environment of the tests.  This is documented in the manual.

# For gen-testsuite-part: ==> try-with-serial-tests <==
. test-init.sh

show_info ()
{
  if test x"$am_serial_tests" != x"yes"; then
     cat foo.log
     cat test-suite.log
  else
     :
  fi
}

mkdir SrcDir
mv [!S]* SrcDir
mkdir BuildDir
cd SrcDir

cat >> configure.ac << 'END'
AC_OUTPUT
END

cat > Makefile.am << 'END'
TESTS = foo.test
END

cat > foo.test <<'END'
#!/bin/sh
echo "foo.test: srcdir='$srcdir'"
test x"$srcdir" = x"../SrcDir"
END
chmod a+x foo.test

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

cd ../BuildDir
../SrcDir/configure

$MAKE check || { show_info; exit 1; }
show_info

:
