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

# Check parallel-tests features: runtime redefinition of $(TEST_SUITE_LOG)
# for the recheck target.

. test-init.sh

cat >> configure.ac <<'END'
AC_OUTPUT
END

cat > Makefile.am <<'END'
TESTS = foofoo.test barbar.test bazbaz.test
END

cat > foofoo.test <<'END'
#! /bin/sh
echo "this is $0"
exit 0
END

cat > barbar.test <<'END'
#! /bin/sh
echo "this is $0"
exit 99
END

cat > bazbaz.test <<'END'
#! /bin/sh
echo "this is $0"
exit ${BAZ_EXIT_STATUS-1}
END

chmod a+x *.test

unset BAZ_EXIT_STATUS

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

# Filter make output before grepping it, for make implementations that,
# like Solaris' one, print the whole of the failed recipe on failure.
filter_stdout ()
{
  grep -v 'bases=.*;' stdout > t && mv -f t stdout
}

./configure
run_make -O -e FAIL check

using_gmake || $sleep # Required by BSD make.

chmod a-rw test-suite.log
run_make -O -e FAIL TEST_SUITE_LOG=my.log recheck
ls -l
filter_stdout
count_test_results total=2 pass=0 fail=1 skip=0 xfail=0 xpass=0 error=1
for x in stdout my.log; do
  $FGREP foofoo $x && exit 1
  $FGREP barbar $x
  $FGREP bazbaz $x
done

using_gmake || $sleep # Required by BSD make.

chmod a-rw my.log
run_make -O -e FAIL BAZ_EXIT_STATUS=0 TEST_SUITE_LOG=my2.log recheck
ls -l
count_test_results total=2 pass=1 fail=0 skip=0 xfail=0 xpass=0 error=1
filter_stdout
$FGREP foofoo stdout && exit 1
$FGREP barbar stdout
$FGREP bazbaz stdout
$FGREP foofoo my2.log && exit 1
$FGREP barbar my2.log
$FGREP bazbaz my2.log && exit 1

chmod u+r test-suite.log my.log
$FGREP bazbaz test-suite.log
$FGREP bazbaz my.log

:
