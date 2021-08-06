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

# Check that recursion on user-defined targets can be made to work
# with "deeply" nested uses of $(SUBDIRS).

. test-init.sh

cat >> configure.ac <<'END'
AC_CONFIG_FILES([
    sub1/Makefile
    sub1/sub2/Makefile
    sub1/sub2/sub3/Makefile
    sub1/sub2/sub3/sub4/Makefile
])
AM_EXTRA_RECURSIVE_TARGETS([foo])
AC_OUTPUT
END

dirs='sub1 sub1/sub2 sub1/sub2/sub3 sub1/sub2/sub3/sub4'

mkdir $dirs

cat > Makefile.am <<'END'
SUBDIRS = sub1

foo-local:
	cp sub1/foo foo
MOSTLYCLEANFILES = foo

.PHONY: test
test:
	echo 'It works!' > exp
	diff exp foo
	diff exp sub1/foo
	test ! -f sub1/sub2/foo
	test ! -f sub1/sub2/sub3/foo
	diff exp sub1/sub2/sub3/sub4/foo
	rm -f exp

all-local: foo
check-local: test
END

cat > sub1/Makefile.am <<'END'
SUBDIRS = sub2
foo-local:
	test ! -f sub2/sub3/foo
	cp sub2/sub3/sub4/foo foo
MOSTLYCLEANFILES = foo
END

# Here we deliberately lack an explicit definition the 'foo-local'
# target; that shouldn't stop 'foo' recursion into subdirectory
# 'sub3/sub4'.
echo SUBDIRS = sub3 > sub1/sub2/Makefile.am
echo SUBDIRS = sub4 > sub1/sub2/sub3/Makefile.am

cat > sub1/sub2/sub3/sub4/Makefile.am <<'END'
foo-local:
	echo 'It works!' > foo
MOSTLYCLEANFILES = foo
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE

for d in $dirs; do
  $FGREP foo-am $d/Makefile.in || exit 1
  case $d in
    */sub4);;
    *) $FGREP foo-recursive $d/Makefile.in || exit 1;;
  esac
done

./configure

$MAKE foo
$MAKE test

$MAKE distcheck

:
