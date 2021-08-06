#! /bin/sh
# Copyright (C) 2012-2018 Free Software Foundation, Inc.
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

# Check that user recursion works with "make -k".

. test-init.sh

cat >> configure.ac <<'END'
AM_EXTRA_RECURSIVE_TARGETS([foo])
AC_CONFIG_FILES([
    sub1/Makefile
    sub1/subsub1/Makefile
    sub2/Makefile
    sub2/subsub2/Makefile
    sub3/Makefile
])
FAIL='@echo "FAIL $@ in `pwd`"; exit 1'
PASS='@echo "PASS $@ in `pwd`"; : > foo'
AC_SUBST([FAIL])
AC_SUBST([PASS])
AC_OUTPUT
END

mkdir sub1 sub1/subsub1 sub2 sub2/subsub2 sub3

cat > Makefile.am <<'END'
SUBDIRS = sub1 . sub2 sub3
foo-local:; @FAIL@
END

cat > sub1/Makefile.am <<'END'
SUBDIRS = subsub1
foo-local:; @PASS@
END

cat > sub2/Makefile.am <<'END'
SUBDIRS = subsub2
foo-local:; @FAIL@
END

echo 'foo-local:; @FAIL@' > sub1/subsub1/Makefile.am
echo 'foo-local:; @PASS@' > sub2/subsub2/Makefile.am
echo 'foo-local:; @PASS@' > sub3/Makefile.am

$ACLOCAL
$AUTOCONF
$AUTOMAKE

./configure

cat > exp <<END
./sub1/foo
./sub2/subsub2/foo
./sub3/foo
END

as_expected ()
{
  find . -name foo > t || { cat t; exit 1; }
  LC_ALL=C sort t > got
  cat exp
  cat got
  diff exp got
}

# Without "-k", we fail in 'sub1/subsub1', and do nothing else.
# So, no 'foo' file gets created.
$MAKE foo && exit 1
find . -name foo | grep . && exit 1

if using_gmake; then
  $MAKE -k foo && exit 1
  as_expected
  $MAKE --keep-going foo && exit 1
  as_expected
else
  # Don't trust the exit status of 'make -k' for non-GNU makes.
  $MAKE -k foo || :
  as_expected
fi

:
