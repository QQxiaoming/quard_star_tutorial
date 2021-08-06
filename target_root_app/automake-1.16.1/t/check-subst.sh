#! /bin/sh
# Copyright (C) 2008-2018 Free Software Foundation, Inc.
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

# Check @substituted@ TESTS.
# Note that in this test, we rely on the .test extension for the
# substituted names: this is necessary for the parallel harness.
# See also sister test 'check-subst-prog.sh'.

# For gen-testsuite-part: ==> try-with-serial-tests <==
. test-init.sh

cat >> configure.ac << 'END'
AC_SUBST([script_tests], ['subst-pass-script.sh subst-xfail-script.sh'])
AC_SUBST([xfail_tests], ['xfail-script.test subst-xfail-script.sh'])
AC_OUTPUT
END

cat > Makefile.am << 'END'
TESTS = pass-script.test xfail-script.test @script_tests@
XFAIL_TESTS = @xfail_tests@
END

if test x"$am_serial_tests" != x"yes"; then
  unindent >> Makefile.am <<'END'
    TEST_EXTENSIONS = .sh .test
    SH_LOG_COMPILER = $(SHELL)
END
fi

cat > pass-script.test <<'END'
#! /bin/sh
exit 0
END
cat > xfail-script.test <<'END'
#! /bin/sh
exit 1
END
cp pass-script.test subst-pass-script.sh
cp xfail-script.test subst-xfail-script.sh
chmod a+x pass-script.test xfail-script.test

if test x"$am_serial_tests" = x"yes"; then
  chmod a+x subst-pass-script.sh subst-xfail-script.sh
fi

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

for vpath in false : ; do

  if $vpath; then
    mkdir build
    cd build
    srcdir=..
  else
    srcdir=.
  fi

  $srcdir/configure
  $MAKE all
  $MAKE check
  if test x"$am_serial_tests" != x"yes"; then
    ls -l
    test -f pass-script.log
    test -f xfail-script.log
    test -f subst-pass-script.log
    test -f subst-xfail-script.log
  fi
  $MAKE distclean
  cd $srcdir

done

:
