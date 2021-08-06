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

# Make sure that --program-transform works even when multiple files are
# collapsed.

required=cc
. test-init.sh

cat >>configure.ac <<'END'
AC_PROG_CC
AC_OUTPUT
END

cat >Makefile.am <<'EOF'
bin_PROGRAMS = p1 p2
bin_SCRIPTS = s1.sh s2.sh
man_MANS = m1.1 m2.1

test-install: install
	test -f inst/bin/p$(EXEEXT)
	test -f inst/bin/s.sh
	test -f inst/man/man1/m.1

test-install-foo: install
	test -f inst/bin/foo$(EXEEXT)
	test -f inst/bin/foo
	test -f inst/man/man1/foo.1
	test ! -f inst/bin/p1$(EXEEXT)
	test ! -f inst/bin/p2$(EXEEXT)
	test ! -f inst/bin/s1.sh
	test ! -f inst/bin/s2.sh
	test ! -f inst/man/man/m1.1
	test ! -f inst/man/man/m2.1
EOF

cat >p1.c <<'EOF'
int
main ()
{
  return 0;
}
EOF

cp p1.c p2.c

: > s1.sh
: > s2.sh
: > m1.1
: > m2.1

$ACLOCAL
$AUTOCONF
$AUTOMAKE

cwd=$(pwd) || fatal_ "getting current working directory"

./configure --program-transform-name='s/[12]//' --prefix "$cwd/inst" \
                                                --mandir "$cwd/inst/man"
$MAKE
$MAKE test-install
$MAKE uninstall
test $(find inst -type f -print | wc -l) -eq 0

# Also squash all file types in question.

# On newer Cygwin versions, that won't work, likely due to overly
# aggressive appending of '.exe' suffix when copying/renaming Windows
# executables.  So let's skip this part of the test if we detect the
# faulty heuristic is present.  See also:
# <https://lists.gnu.org/archive/html/automake-patches/2010-08/msg00153.html>
# <http://thread.gmane.org/gmane.os.cygwin/119380>
echo Foo > foo
echo Bar > bar.exe
chmod a+x foo bar.exe
cp foo bar && cmp foo bar \
  || skip_ "your Cygwin is too aggressive in tweaking '.exe' suffixes"

./configure --program-transform-name='s/.*/foo/' --prefix "$cwd/inst" \
                                                 --mandir "$cwd/inst/man"
$MAKE
$MAKE test-install-foo
$MAKE uninstall
test $(find inst -type f -print | wc -l) -eq 0

:
