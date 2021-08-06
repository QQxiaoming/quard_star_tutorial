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

# Check @substituted@ TESTS, some of which are also PROGRAMS.
# See also sister test 'check-subst.sh'.

# For gen-testsuite-part: ==> try-with-serial-tests <==
required='cc native'
. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AC_SUBST([script_tests],
         ['subst-pass-script.test subst-xfail-script.test'])
AC_SUBST([prog_tests],
         ['subst-pass-prog.test$(EXEEXT) subst-xfail-prog.test$(EXEEXT)'])
AC_SUBST([xfail_tests],
         ['xfail-script.test subst-xfail-script.test]dnl
         [ xfail-prog$(EXEEXT) subst-xfail-prog.test$(EXEEXT)'])
AC_OUTPUT
END

cat > Makefile.am << 'END'
TESTS = pass-script.test xfail-script.test @script_tests@ $(check_PROGRAMS)
XFAIL_TESTS = @xfail_tests@
check_PROGRAMS = pass-prog xfail-prog @prog_tests@
EXTRA_PROGRAMS = subst-pass-prog.test subst-xfail-prog.test
END

cat > pass-script.test <<'END'
#! /bin/sh
exit 0
END
cat > xfail-script.test <<'END'
#! /bin/sh
exit 1
END
chmod a+x pass-script.test xfail-script.test
cp pass-script.test subst-pass-script.test
cp xfail-script.test subst-xfail-script.test

cat > pass-prog.c <<'END'
int main (void) { return 0; }
END
cat > xfail-prog.c <<'END'
#include <stdlib.h>
int main (void) { return EXIT_FAILURE; }
END
# The .test extension is removed for the default source file name:
cp pass-prog.c subst-pass-prog.c
cp xfail-prog.c subst-xfail-prog.c

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure
$MAKE all
$MAKE check
$MAKE distclean

mkdir build
cd build
../configure
$MAKE all
$MAKE check
$MAKE distclean

:
