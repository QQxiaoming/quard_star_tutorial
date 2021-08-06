#! /bin/sh
# Copyright (C) 2002-2018 Free Software Foundation, Inc.
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

# Make sure 'check:' honors $(BUILT_SOURCES).
# PR/359.

# For gen-testsuite-part: ==> try-with-serial-tests <==
. test-init.sh

cat >> configure.ac << 'END'
AC_CONFIG_FILES([dir/Makefile])
AC_OUTPUT
END

mkdir dir

cat > Makefile.am << 'END'
BUILT_SOURCES = command1.inc
SUBDIRS = dir
TESTS = subrun.sh
subrun.sh:
	(echo '#! /bin/sh'; cat command1.inc) > $@
	chmod +x $@
command1.inc:
	echo 'dir/echo.sh' > $@
CLEANFILES = subrun.sh command1.inc
END

cat > dir/Makefile.am << 'END'
BUILT_SOURCES = command2.inc
check_SCRIPTS = echo.sh
echo.sh:
## The next line ensures that command1.inc has been built before
## recurring into the subdir.
	test -f ../command1.inc
	(echo '#! /bin/sh'; cat command2.inc) > $@
	chmod +x $@
command2.inc:
	echo 'echo Hello' > $@
CLEANFILES = echo.sh command2.inc
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a
./configure --prefix "$(pwd)/inst"

run_make -O check
grep '^PASS: subrun\.sh *$' stdout
grep 'PASS.*echo\.sh' stdout && exit 1

$MAKE distcheck

:
