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

# parallel-tests:
#  - LOG_DRIVER variables can be AC_SUBST'd

. test-init.sh

mkdir test-drivers

cp "$am_testaux_srcdir"/trivial-test-driver test-drivers/triv \
  || fatal_ "failed to fetch auxiliary script trivial-test-driver"
cp "$am_scriptdir"/test-driver test-drivers/dflt \
  || fatal_ "failed to fetch auxiliary script test-driver"

cat >> configure.ac <<'END'
AC_SUBST([LOG_DRIVER],      ['${SHELL} test-drivers/triv'])
AC_SUBST([TEST_LOG_DRIVER], ['${SHELL} test-drivers/dflt'])
AC_SUBST([SH_LOG_DRIVER],   ['${my_drv}'])
AC_OUTPUT
END

cat > Makefile.am <<'END'
TEST_EXTENSIONS = .test .sh
my_drv = $(SHELL) test-drivers/dflt
TESTS = foo bar.test baz.sh
XFAIL_TESTS = baz.sh
.PHONY: check-autodefs
check-autodefs:
### For debugging.
	@echo      LOG_DRIVER = $(LOG_DRIVER)
	@echo TEST_LOG_DRIVER = $(TEST_LOG_DRIVER)
	@echo   SH_LOG_DRIVER = $(SH_LOG_DRIVER)
### Checks here.
	@echo ' ' $(LOG_DRIVER)      ' ' | grep ' test-drivers/triv '
	@echo ' ' $(TEST_LOG_DRIVER) ' ' | grep ' test-drivers/dflt '
	@echo ' ' $(SH_LOG_DRIVER)   ' ' | grep ' test-drivers/dflt '
END

$ACLOCAL
$AUTOMAKE
$AUTOCONF

./configure

cat > foo <<'END'
#!/bin/sh
echo "PASS: from $0"
exit 1 # exit status should be ignored by the trivial-test-driver.
END

cat > bar.test <<'END'
#!/bin/sh
exit 77
END

cat > baz.sh <<'END'
#!/bin/sh
exit 1
END

chmod a+x foo bar.test baz.sh

$MAKE check-autodefs
run_make -O -e IGNORE check
cat test-suite.log
cat foo.log
cat bar.log
cat baz.log
test $am_make_rc -eq 0
count_test_results total=3 pass=1 fail=0 skip=1 xfail=1 xpass=0 error=0

:
