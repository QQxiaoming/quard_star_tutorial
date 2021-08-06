#! /bin/sh
# Copyright (C) 2010-2018 Free Software Foundation, Inc.
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

# Test that POSIX variable expansion '$(var:str=rpl)' works when used
# with the TESTS special variable.

# For gen-testsuite-part: ==> try-with-serial-tests <==
. test-init.sh

cat >> configure.ac << 'END'
AC_OUTPUT
END

cat > Makefile.am << 'END'
t1 = foo1 foo2
t2 = barx
t3 = bar2

foo2.test barz:
	(echo '#!/bin/sh' && echo 'exit 0') > $@ && chmod a+x $@
CLEANFILES = foo2.test barz # For FreeBSD make.

# Also try an empty match suffix, to ensure that the ':=' in there is
# not confused by the parser with an unportable assignment operator.
TESTS = $(t1:=.test) $(t2:x=y) $(t3:2=z)

EXTRA_DIST = $(TESTS)
MOSTLYCLEANFILES = *.out
END

cat > foo1.test <<'END'
#!/bin/sh
touch foo1.out
test x"${TESTSUITE_OK-no}" = x"yes"
END
sed 's/foo1/bary/g' foo1.test > bary
chmod +x foo1.test bary

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure
run_make -M -e FAIL check
ls -l
grep '^FAIL: foo1\.test *$' output
grep '^PASS: foo2\.test *$' output
grep '^FAIL: bary *$' output
grep '^PASS: barz *$' output
test -f foo1.out
test -f bary.out

TESTSUITE_OK=yes $MAKE distcheck

:
