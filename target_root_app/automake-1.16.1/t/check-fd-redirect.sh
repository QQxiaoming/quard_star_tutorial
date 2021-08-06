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

# Simple Tests support: redirection of file descriptors with
# AM_TESTS_FD_REDIRECT.
# See also related test 'parallel-tests-fd-redirect.sh'.

# For gen-testsuite-part: ==> try-with-serial-tests <==
. test-init.sh

cat >> configure.ac << 'END'
AC_OUTPUT
END

cat > Makefile.am <<'END'
TESTS = foo.test
AM_TESTS_FD_REDIRECT = 3<three 4>four 5>>five 7<&0 8>&1 9>&2
END

echo '3333' > three
chmod a-w three

: > foo.test
chmod a+x foo.test

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a
./configure

do_check ()
{
  cat foo.test # For debugging.
  echo 'this line will be removed' > four
  echo 'this line will not be removed' > five
  st=0; echo 'ok ok ok' | run_make -O -E -e IGNORE check || st=$?
  cat four
  test x"$am_serial_tests" = x"yes" || cat foo.log
  test $st -eq 0
  grep '[ /]foo\.test: foofoofoo$' stdout
  grep '[ /]foo\.test: barbarbar$' stderr
  grep 'this line' four && exit 1
  grep '^3333$' four
  grep '^this line will not be removed$' five
  grep '^ok ok ok$' five
  $EGREP '(foofoofoo|barbarbar|3333|ok ok ok|this line)' foo.log && exit 1
  :
}

# Try using both shell script and a perl script as the test, for
# better coverage.

cat > foo.test <<'END'
#! /bin/sh
set -e

read FOO <&3
test 3333 -eq "$FOO"
echo "$FOO" >&4

grep '^ok ok ok$' <&7 >&5

echo " " $0: foofoofoo >&8
echo " " $0: barbarbar >&9
END

do_check

echo "#! $PERL -w" > foo.test
cat >> foo.test <<'END'
use warnings FATAL => 'all';
use strict;

open (FD3, "<&=3") or die "opening FD3: $!";
open (FD4, ">&=4") or die "opening FD4: $!";
open (FD5, ">&=5") or die "opening FD5: $!";
open (FD7, "<&=7") or die "opening FD7: $!";
open (FD8, ">&=8") or die "opening FD8: $!";
open (FD9, ">&=9") or die "opening FD9: $!";

chomp (my $FOO = <FD3>);
die "$FOO != 3333" if not $FOO eq "3333";
print FD4 "$FOO\n";

chomp ($_ = <FD7>);
die "$_ != 'ok ok ok'" if not $_ eq 'ok ok ok';
print FD5 "$_\n";

print FD8 "  $0: foofoofoo\n";
print FD9 "  $0: barbarbar\n";
END

do_check

:
