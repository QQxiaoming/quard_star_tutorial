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

# Check parallel-tests features:
#  - $(TESTS_LOGS) can end up holding duplicated entries
# Such behaviour can be useful for projects that want to verify that
# different test scripts don't end up sharing the same log file.
# (Automake itself is such a project, BTW ;-)

. test-init.sh

cat >> configure.ac <<END
AC_OUTPUT
END

cat > Makefile.am << 'END'
TEST_EXTENSIONS = .sh .test .bar
TESTS = foo.sh mu.test foo.test foo.bar
.PHONY: verify
verify:
	is $(TEST_LOGS) == foo.log mu.log foo.log foo.log
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure
$MAKE verify

:
