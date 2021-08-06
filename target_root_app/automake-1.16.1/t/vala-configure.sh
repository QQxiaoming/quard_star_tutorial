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

# Test AM_PROG_VALAC.

. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AM_PROG_VALAC([1.2.3])
AC_OUTPUT
END

cat > Makefile.am << 'END'
has-valac:
	case '$(VALAC)' in */bin/valac) exit 0;; *) exit 1;; esac
no-valac:
	test x'$(VALAC)' = x'valac'
END

mkdir bin
cat > bin/valac << 'END'
#! /bin/sh
if test "x$1" = x--version; then
  echo "${vala_version-1.2.3}"
fi
exit 0
END
chmod +x bin/valac

cat > bin/valac.old << 'END'
#! /bin/sh
if test "x$1" = x--version; then
  echo 0.1
fi
exit 0
END
chmod +x bin/valac.old

PATH=$(pwd)/bin$PATH_SEPARATOR$PATH; export PATH

# Avoid interferences from the environment.
unset VALAC vala_version

$ACLOCAL
$AUTOMAKE -a
$AUTOCONF

st=0; ./configure 2>stderr || st=$?
cat stderr >&2
grep 'WARNING.*vala' stderr && exit 1
test $st -eq 0
$MAKE has-valac

st=0; vala_version=99.9 ./configure 2>stderr || st=$?
cat stderr >&2
grep 'WARNING.*vala' stderr && exit 1
test $st -eq 0
$MAKE has-valac

st=0; vala_version=0.1.2 ./configure 2>stderr || st=$?
cat stderr >&2
test $st -eq 0
grep '^configure: WARNING: no proper vala compiler found' stderr
$MAKE no-valac

st=0; ./configure VALAC="$(pwd)/bin/valac.old" 2>stderr || st=$?
cat stderr >&2
test $st -eq 0 || exit 1
grep '^configure: WARNING: no proper vala compiler found' stderr
$MAKE no-valac

sed 's/^\(AM_PROG_VALAC\).*/\1([1], [: > ok], [: > ko])/' <configure.ac >t
mv -f t configure.ac
rm -rf autom4te*.cache
$ACLOCAL
$AUTOCONF

st=0; ./configure 2>stderr || st=$?
cat stderr >&2
grep 'WARNING.*vala' stderr && exit 1
test -f ok
test ! -e ko
$MAKE has-valac
rm -f ok ko

st=0; vala_version=0.1.2 ./configure 2>stderr || st=$?
cat stderr >&2
grep 'WARNING.*vala' stderr && exit 1
test $st -eq 0
test ! -e ok
test -f ko
$MAKE no-valac
rm -f ok ko

st=0; ./configure VALAC="$(pwd)/bin/valac.old" 2>stderr || st=$?
cat stderr >&2
grep 'WARNING.*vala' stderr && exit 1
test $st -eq 0
test ! -e ok
test -f ko
$MAKE no-valac
rm -f ok ko

:
