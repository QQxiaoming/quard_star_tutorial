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
# with the SCRIPTS primary.

. test-init.sh

cat >> configure.ac << 'END'
AC_OUTPUT
END

# We have to be careful with 'test -x' commands on MinGW/MSYS, because
# the file system does not actually have execute permission information.
# Instead, that is emulated by looking at the file content, and returning
# 0 if the file starts with, e.g., a COFF header or with '#!'.
# So we need to create actual scripts in the make rules and in the file
# creation below.

cat > Makefile.am << 'END'
t1 = foo1 foo2
t2 = bar1x bar2
t3 = quu-baz

bar1 bar2:
	(echo '#!/bin/sh' && echo 'exit 0') > $@
quux.pl:
	(echo '#!/bin/perl' && echo '1;') > $@
CLEANFILES = bar1 bar2 quux.pl

# Also try an empty match suffix, to ensure that the ':=' in there is
# not confused by the parser with an unportable assignment operator.
dist_sbin_SCRIPTS = $(t1:=.sh)
libexec_SCRIPTS = $(t2:x=)
nodist_bin_SCRIPTS = $(t3:-baz=x.pl)

check-local: test1 test2
.PHONY: test1 test2
test1:
	ls -l
	test -f bar1
	test -f bar2
	test -f quux.pl
test2: distdir
	ls -l $(distdir)
## The scripts foo1.sh and foo2.sh should be distributed.
	test -f $(distdir)/foo1.sh
	test -f $(distdir)/foo2.sh
## The scripts bar1, bar2 and quux.pl shouldn't be distributed.
	test ! -r $(distdir)/bar1
	test ! -r $(distdir)/bar2
	test ! -r $(distdir)/quux.pl

installcheck-local:
	ls -l $(libexecdir) $(bindir) $(sbindir)
	test -f $(sbindir)/foo1.sh
	test -x $(sbindir)/foo1.sh
	test -f $(sbindir)/foo2.sh
	test -x $(sbindir)/foo2.sh
	test -f $(libexecdir)/bar1
	test -x $(libexecdir)/bar1
	test -f $(libexecdir)/bar2
	test -x $(libexecdir)/bar2
	test -f $(bindir)/quux.pl
	test -x $(bindir)/quux.pl
END

cat > foo1.sh <<'END'
#!/bin/sh
exit 0
END
cp foo1.sh foo2.sh

$ACLOCAL
$AUTOCONF
$AUTOMAKE

cwd=$(pwd) || fatal_ "getting current working directory"
./configure --prefix="$cwd/_inst"
$MAKE
$MAKE test1 test2
$MAKE install
$MAKE installcheck
$MAKE distcheck

:
