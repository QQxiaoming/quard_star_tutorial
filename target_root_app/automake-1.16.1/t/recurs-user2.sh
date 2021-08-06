#! /bin/sh
# Copyright (C) 2012-2018 Free Software Foundation, Inc.
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

# Check that user recursion can be made to work when $(SUBDIRS) are
# not "strictly" nested, as in e.g.:
#    SUBDIRS = src external/lib external/tests
# with no Makefile in 'external'.

. test-init.sh

cat >> configure.ac <<'END'
AC_CONFIG_FILES(
    sub1/Makefile
    sub2/Makefile
dnl There's deliberately no 'sub3/Makefile'.
    sub3/subsub/Makefile
    sub4/Makefile
    sub4/subsub/Makefile
)
AM_EXTRA_RECURSIVE_TARGETS([foo])
AC_OUTPUT
END

mkdir sub1 sub2 sub3 sub4 sub3/subsub sub4/subsub

cat > Makefile.am <<'END'
SUBDIRS = sub1 sub2 sub3/subsub sub4 sub4/subsub

foo-local:
	echo _rootdir_ >foo
MOSTLYCLEANFILES = foo

.PHONY: test
test: foo
	grep _rootdir_ foo
	grep ':sub1:' sub1/foo
	grep ',sub1,' sub1/bar
	test ! -r sub2/foo
	test ! -r sub3/foo
	grep '%sub3/subsub%' sub3/subsub/quux
	test ! -r sub3/subsub/foo
	test ! -r sub4/foo
	grep '=sub4/subsub=' sub4/subsub/foo

check-local: test
END

# A 'foo-local' target with dependencies shouldn't cause problems.
cat > sub1/Makefile.am <<'END'
foo-local: bar
	sed 's/,/:/g' bar >foo
bar:
	echo ',sub1,' >$@
MOSTLYCLEANFILES = foo bar
END

# The lack of a 'foo' target here shouldn't cause any error in
# automake nor in make.
: > sub2/Makefile.am

# The lack of file 'sub3/Makefile.am' shouldn't cause any problem either.
rm -f sub3/Makefile.am

# A 'foo-local' creating a file != 'foo' shouldn't cause any problem.
cat > sub3/subsub/Makefile.am <<'END'
foo-local:
	echo '%sub3/subsub%' >quux
MOSTLYCLEANFILES = quux
END

# No 'foo-local' nor 'foo' target here ...
: > sub4/Makefile.am
# ... should not cause errors, nor cause the 'foo-local' target
# here not to be executed.
cat > sub4/subsub/Makefile.am <<'END'
foo-local:
	echo '=sub4/subsub=' >foo
MOSTLYCLEANFILES = foo
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE

./configure

$MAKE test
$MAKE distcheck

:
