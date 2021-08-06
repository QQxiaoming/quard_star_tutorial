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

# parallel-tests: some checks on console output about testsuite
# progress.

. test-init.sh

cat >> configure.ac << 'END'
AC_OUTPUT
END

cat > Makefile.am << 'END'
XFAIL_TESTS = sub/xpass.test xfail.test error.test
TESTS = $(XFAIL_TESTS) fail.test pass.test a/b/skip.test sub/error2.test
pass.log: fail.log
error.log: pass.log
sub/xpass.log: error.log
sub/error2.log: xfail.log
a/b/skip.log: sub/error2.log
END

cat > exp <<'END'
FAIL: fail.test
PASS: pass.test
ERROR: error.test
XPASS: sub/xpass.test
XFAIL: xfail.test
ERROR: sub/error2.test
SKIP: a/b/skip.test
END

mkdir sub a a/b

cat > pass.test << 'END'
#!/bin/sh
exit 0
END
cp pass.test sub/xpass.test

cat > fail.test << 'END'
#!/bin/sh
exit 1
END

cat > xfail.test << 'END'
#!/bin/sh
# The sleep should ensure expected execution order of tests
# even when make is run in parallel mode.
# Creative quoting below to plase maintainer-check.
sleep '10'
exit 1
END

cat > error.test << 'END'
#!/bin/sh
exit 99
END
cp error.test sub/error2.test

cat > a/b/skip.test << 'END'
#!/bin/sh
exit 77
END

chmod a+x pass.test fail.test xfail.test sub/xpass.test \
          a/b/skip.test error.test sub/error2.test

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

for vpath in : false; do
  if $vpath; then
    mkdir build
    cd build
    srcdir=..
  else
    srcdir=.
  fi
  $srcdir/configure
  run_make -O -e FAIL check
  LC_ALL=C grep '^[A-Z][A-Z]*:' stdout > got
  cat got
  diff $srcdir/exp got
  cd $srcdir
done

:
