#! /bin/sh
# Copyright (C) 2009-2018 Free Software Foundation, Inc.
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

# Check parallel-tests features:
# - suffix rules
# See also sister test 'parallel-tests-suffix-prog.sh'.

. test-init.sh

cat >> configure.ac << 'END'
AC_OUTPUT
END

cat > Makefile.am << 'END'
## Note that automake should not match the '/test' part
## of 'sub/test' as '.test' suffix.
TESTS = foo.chk bar.test baz bli.suff sub/test
TEST_EXTENSIONS = .chk .test
EXTRA_DIST = $(TESTS)
END

mkdir sub

cat >foo.chk << 'END'
#! /bin/sh
exit 0
END
chmod a+x foo.chk
cp foo.chk bar.test
cp foo.chk baz
cp foo.chk bli.suff
cp foo.chk sub/test

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure
$MAKE check
ls -l . sub
test -f foo.log
test -f bar.log
test -f baz.log
test -f bli.suff.log
test -f sub/test.log

$MAKE distcheck

:
