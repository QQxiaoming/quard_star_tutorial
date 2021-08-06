#! /bin/sh
# Copyright (C) 2013-2018 Free Software Foundation, Inc.
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

# Check that uses of the obsolescent AM_PROG_CC_C_O macro doesn't
# cause spurious warnings or errors.  Suggested by Eric Blake.

# We need gcc for for two reasons:
#  1. to ensure our C compiler grasps "-c -o" together.
#  2. to be able to later fake a dumb compiler not grasping that
#     (done with 'cc-no-c-o' script below, which required gcc).
required=gcc
. test-init.sh

echo bin_PROGRAMS = foo > Makefile.am
echo 'int main (void) { return 0; }' > foo.c

cp configure.ac configure.bak

cat > acinclude.m4 <<'END'
AC_DEFUN([AM_TWEAKED_OUTPUT], [
# For debugging.
printf "CC = '%s'\\n" "$CC"
# Make sure that $CC can be used after AM_PROG_CC_C_O.
$CC --version || exit 1
$CC -v || exit 1
# $CC rewrite should only take place on time.
case " $CC " in
  *" compile"*" compile"*) AC_MSG_ERROR([CC rewritten twice]);;
esac
AC_OUTPUT
])
END

# ---

cat configure.bak - > configure.ac << 'END'
dnl It's OK to call AM_PROG_CC_C_O after AC_PROG_CC.
AC_PROG_CC
AM_PROG_CC_C_O
AM_TWEAKED_OUTPUT
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE --add-missing

./configure >stdout || { cat stdout; exit 1; }
cat stdout
if test "$AM_TESTSUITE_SIMULATING_NO_CC_C_O" != no; then
  $EGREP 'understands? -c and -o together.* no$' stdout
else
  $EGREP 'understands? -c and -o together.* yes$' stdout
fi

# No repeated checks please.
test $(grep -c ".*-c['\" ].*-o['\" ]" stdout) -eq 1

$MAKE
$MAKE maintainer-clean
rm -rf autom4te*.cache

# ---

cat configure.bak - > configure.ac << 'END'
dnl It's also OK to call AM_PROG_CC_C_O *before* AC_PROG_CC.
AM_PROG_CC_C_O
AC_PROG_CC
AM_TWEAKED_OUTPUT
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE --add-missing

./configure >stdout || { cat stdout; exit 1; }
cat stdout
if test "$AM_TESTSUITE_SIMULATING_NO_CC_C_O" != no; then
  $EGREP 'understands? -c and -o together.* no$' stdout
else
  $EGREP 'understands? -c and -o together.* yes$' stdout
fi

# Repeated checks are OK in this case, but should be cached.
test $(grep ".*-c['\" ].*-o['\" ]" stdout \
        | $FGREP -v ' (cached) ' | wc -l) -eq 1

$MAKE
$MAKE maintainer-clean
rm -rf autom4te*.cache

# ---

cat configure.bak - > configure.ac << 'END'
dnl It's also OK to call AM_PROG_CC_C_O *without* AC_PROG_CC.
AM_PROG_CC_C_O
AM_TWEAKED_OUTPUT
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE --add-missing

# Make sure the compiler doesn't understand '-c -o'
CC=$am_testaux_builddir/cc-no-c-o; export CC

./configure >stdout || { cat stdout; exit 1; }
cat stdout
$EGREP 'understands? -c and -o together.* no$' stdout
# No repeated checks please.
test $(grep -c ".*-c['\" ].*-o['\" ]" stdout) -eq 1
$MAKE

:
