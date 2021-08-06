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

# Check that the testsuite driver copes well with unreadable '.log'
# and '.trs' files.

. test-init.sh

: > t
chmod a-r t && test ! -r t || skip_ "you can still read unreadable files"
rm -f t

cat >> configure.ac << 'END'
AC_OUTPUT
END

cat > Makefile.am << 'END'
TESTS = foo.test bar.test
END

cat > foo.test << 'END'
#! /bin/sh
echo foofoofoo
exit 0
END

cat > bar.test << 'END'
#! /bin/sh
echo barbarbar
exit 77
END

chmod a+x foo.test bar.test

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure

for files in \
  'foo.log bar.log' \
  'foo.trs bar.trs' \
  'foo.trs bar.log' \
  'foo.log bar.trs' \
; do
  $MAKE check
  rm -f test-suite.log
  chmod a-r $files
  $MAKE test-suite.log || { ls -l; exit 1; }
  ls -l
  grep '^foofoofoo$' foo.log
  grep '^:test-result: PASS' foo.trs
  grep '^barbarbar$' bar.log
  grep '^:test-result: SKIP' bar.trs
  grep '^SKIP: bar' test-suite.log
  grep '^barbarbar$' test-suite.log
  $EGREP ':.*foo|foofoofoo' test-suite.log && exit 1
  : For shells with busted 'set -e'.
done

:
