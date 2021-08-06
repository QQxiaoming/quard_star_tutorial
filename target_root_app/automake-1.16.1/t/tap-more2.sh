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

# More on TAP support:
#  - more LOG_COMPILER at once for TAP tests
#  - binary programs in $(TESTS)
#  - interactions with 'check_*' variables

required='cc native'
. test-init.sh

fetch_tap_driver

cat >> configure.ac <<END
AC_PROG_CC
AC_OUTPUT
END

cat > Makefile.am << 'END'
TEST_EXTENSIONS = .sh .tap

tap_driver = $(srcdir)/tap-driver

LOG_DRIVER = $(tap_driver)
SH_LOG_DRIVER = $(tap_driver)
TAP_LOG_DRIVER = $(tap_driver)

TAP_LOG_COMPILER = cat
SH_LOG_COMPILER = $(SHELL)
LOG_COMPILER =

EXTRA_DIST = baz.tap

check_SCRIPTS = bar.sh
bar.sh: Makefile
	echo '#!/bin/sh' > $@-t
	echo 'echo 1..1' >> $@-t
	echo 'echo "not ok 1 # TODO"' >> $@-t
	chmod a-w $@-t && mv -f $@-t $@
CLEANFILES = bar.sh

check_PROGRAMS = foo-test
foo_test_SOURCES = foo.c

TESTS = foo-test $(check_SCRIPTS) baz.tap

EXTRA_DIST += tap-driver
END

cat > foo.c <<'END'
#include <stdio.h>
int main (void)
{
  printf ("1..1\n");
  printf ("ok 1\n");
  return 0;
}
END

cat > baz.tap << 'END'
1..1
ok 1 # SKIP
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE

./configure

for target in check distcheck; do
  run_make -O $target
  count_test_results total=3 pass=1 fail=0 xpass=0 xfail=1 skip=1 error=0
done

:
