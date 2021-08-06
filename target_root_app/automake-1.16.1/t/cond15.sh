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

# Regression test for conditionally defined overriding of automatic rules.

. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AM_CONDITIONAL([COND1], [true])
AM_CONDITIONAL([COND2], [true])
END

cat > Makefile.am << 'END'
if COND1
if COND2
bin_SCRIPTS = helldl
helldl$(EXEEXT):
	rm -f $@
	echo '#! /bin/sh' > $@
	echo '-dlopen is unsupported' >> $@
	chmod +x $@
endif
else
if COND2
else
bin_SCRIPTS = helldl
helldl$(EXEEXT):
	rm -f $@
	echo '#! /bin/sh' > $@
	echo '-dlopen is unsupported' >> $@
	chmod +x $@
endif
endif

bin_PROGRAMS = helldl
END

$ACLOCAL
$AUTOMAKE

$FGREP helldl Makefile.in # For debugging.
num1=$($FGREP -c 'helldl$(EXEEXT):' Makefile.in)
num2=$($FGREP -c '@COND1_FALSE@@COND2_TRUE@helldl$(EXEEXT):' Makefile.in)
test $num1 -eq 4
test $num2 -eq 1

:
