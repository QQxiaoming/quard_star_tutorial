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

# Check parallel-tests features: runtime redefinition of:
#  - $(TEST_SUITE_LOG) and $(TESTS)
#  - $(TEST_SUITE_LOG) and $(TEST_LOGS)

. test-init.sh

cat >> configure.ac <<'END'
AC_OUTPUT
END

cat > Makefile.am <<'END'
TESTS = pass.test pass2.test skip.test skip2.test fail.test
END

cat > pass.test <<'END'
#! /bin/sh
exit 0
END

cp pass.test pass2.test

cat > skip.test <<'END'
#! /bin/sh
echo "% skipped test %"
exit 77
END

cp skip.test skip2.test

cat > fail.test <<'END'
#! /bin/sh
exit 1
END

chmod a+x *.test

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure

for test_list_override in \
  'TESTS=pass.test skip.test' \
  'TEST_LOGS=pass.log skip.log'
do
  run_make -O TEST_SUITE_LOG=partial.log "$test_list_override" check
  ls -l
  count_test_results total=2 pass=1 fail=0 skip=1 xfail=0 xpass=0 error=0
  cat pass.log
  cat skip.log
  cat partial.log
  test ! -e test-suite.log
  test ! -e pass2.log
  test ! -e skip2.log
  test ! -e fail.log
  grep '^PASS: pass\.test$' stdout
  grep '^SKIP: skip\.test$' stdout
  $FGREP 'SKIP: skip' partial.log
  $FGREP '% skipped test %' partial.log
  $EGREP '(pass2|skip2|fail)\.test' stdout && exit 1
  $EGREP '(pass2|skip2|fail)' partial.log && exit 1
  rm -f *.log
done

:
