#! /bin/sh
# Copyright (C) 2001-2018 Free Software Foundation, Inc.
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

# Test to make sure Automake supports implicit rules with dot-less
# extensions.  Se also related "grepping" test 'suffix6.sh'.

required=GNUmake # Other makes might not grok dot-less suffix rules.
. test-init.sh

cat >> configure.ac << 'END'
# $(LINK) is not defined automatically by Automake, since the *_SOURCES
# variables don't contain any known extension (.c, .cc, .f, ...).
# So we need this hack -- but since such an hack can also serve as a
# mild stress test, that's ok.
AC_SUBST([LINK], ['cat >$@'])
AC_SUBST([OBJEXT], [oOo])
AC_SUBST([EXEEXT], [.XxX])
AC_OUTPUT
END

cat > Makefile.am << 'END'
SUFFIXES = a b c .$(OBJEXT)
bin_PROGRAMS = foo
foo_SOURCES = fooa
ab:
	{ echo '=ab=' && cat $<; } >$@
bc:
	{ echo '=bc=' && cat $<; } >$@
c.$(OBJEXT):
	{ echo '=b.obj=' && cat $<; } >$@
test:
	: For debugging.
	ls -l
	: Implicit intermediate files should be removed by GNU make ...
	test ! -r foob
	test ! -r fooc
	: ... but object files should not.
	cat foo.$(OBJEXT)
	: For debugging.
	cat foo.XxX
	: Now check that the chain of implicit rules has been executed
	: completely and in the correct order.
	(echo =b.obj= && echo =bc= && echo =ab= && echo =src=) > exp
	diff exp foo.XxX
	rm -f exp
.PHONY: test
check-local: test
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE

./configure

echo =src= > fooa

$MAKE
$MAKE test
$MAKE distcheck

:
