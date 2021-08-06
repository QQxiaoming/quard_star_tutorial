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

# parallel-tests support: redirection of file descriptors with
# AM_TESTS_FD_REDIRECT, even when using tests without suffix.
# The sister 'parallel-tests-fd-redirect-exeext.sh' do a similar
# check for tests that are binary executables.
# See also the more generic test 'check-fd-redirect.sh'.

. test-init.sh

cat >> configure.ac << 'END'
AC_OUTPUT
END

cat > Makefile.am << 'END'
AM_TESTS_FD_REDIRECT = 9>&1
TEST_EXTENSIONS = .test .sh
TESTS = foo.sh bar
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

cat > foo.sh <<'END'
#!/bin/sh
echo " foofoofoo" >&9
END
chmod a+x foo.sh

cat > bar <<'END'
#!/bin/sh
echo " barbarbar" >&9
END

chmod a+x foo.sh bar

./configure

# Sanity checks.
st=0
grep '^bar\.log:.*bar' Makefile || st=1
grep '^foo\.log:' Makefile && st=1
test $st -eq 0 || fatal_ "doesn't cover expected code paths"

run_make -O -e IGNORE check
cat foo.log
cat bar.log
test $am_make_rc -eq 0
grep "^ foofoofoo$" stdout
grep "^ barbarbar$" stdout
$EGREP '(foofoofoo|barbarbar)' *.log && exit 1

:
