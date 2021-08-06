#! /bin/sh
# Copyright (C) 2008-2018 Free Software Foundation, Inc.
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

# The install rule should honor failures of the install program.
# Some of these are already caught by 'instmany.sh'.

# This test has a few sister tests, for java, info, libtool.

required=cc
. test-init.sh

cat >>configure.ac <<END
AC_PROG_CC
AM_PROG_AR
AC_PROG_RANLIB
AM_PATH_LISPDIR
AC_OUTPUT
END

cat >Makefile.am <<'END'
bin_PROGRAMS = prog1 prog2 prog3
nobase_bin_PROGRAMS = progn1 progn2 progn3
lisp_LISP = lisp1.el lisp2.el lisp3.el
nobase_lisp_LISP = lispn1.el lispn2.el lispn3.el
lib_LIBRARIES = lib1.a lib2.a lib3.a
nobase_lib_LIBRARIES = libn1.a libn2.a libn3.a
unreadable-prog:
	chmod a-r prog1$(EXEEXT)
readable-prog:
	chmod a+r prog1$(EXEEXT)
unreadable-progn:
	chmod a-r progn1$(EXEEXT)
readable-progn:
	chmod a+r progn1$(EXEEXT)
END

for n in 1 2 3; do
  echo "int main () { return 0; }" > prog$n.c
  echo "int main () { return 0; }" > progn$n.c
  echo "int foo$n () { return 0; }" > lib$n.c
  echo "int foon$n () { return 0; }" > libn$n.c
  echo >lisp$n.el
  echo >lispn$n.el
done

$ACLOCAL
$AUTOCONF
$AUTOMAKE --add-missing

instdir=$(pwd)/inst || fatal_ "getting current working directory"
./configure --prefix="$instdir"
$MAKE

$MAKE install
$MAKE uninstall

for file in lib1.a libn1.a
do
  chmod a-r $file
  test ! -r $file || skip_ "cannot drop file read permissions"
  $MAKE install-exec && exit 1
  chmod u+r $file
done

$MAKE unreadable-prog
$MAKE install-exec && exit 1
$MAKE readable-prog

$MAKE unreadable-progn
$MAKE install-exec && exit 1
$MAKE readable-progn

if ! grep "^EMACS = no" Makefile; then
  for file in lisp1.el lisp1.elc; do
    chmod a-r $file
    $MAKE install-data && exit 1
    chmod u+r $file
  done
fi

:
