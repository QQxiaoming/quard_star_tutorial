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

# Test for valid prefix/primary combinations.
# See also test 'primary-prefix-invalid-couples.sh'.

. test-init.sh

cat >> configure.ac <<'END'
AC_PROG_CC
AM_PROG_AR
AC_PROG_RANLIB
AC_PROG_LIBTOOL
AM_PATH_PYTHON
AM_PATH_LISPDIR
END

# Fake libtool availability.
: > ltmain.sh
: > config.sub
: > config.guess
cat > acinclude.m4 <<'END'
AC_DEFUN([AC_PROG_LIBTOOL],
         [AC_SUBST([LIBTOOL], [:])])
END

# Other required files.
echo '@setfilename foo.info' > foo.texi
: > texinfo.tex
: > py-compile
: > ar-lib

# Setup Makefile.am.

: > Makefile.am

for p in bin sbin libexec pkglibexec; do
  echo "${p}_PROGRAMS = prog-$p" >> Makefile.am
done

for p in lib pkglib; do
  echo "${p}_LIBRARIES = libs-$p.a" >> Makefile.am
  echo "${p}_LTLIBRARIES = libd-$p.la" >> Makefile.am
done

for p in bin sbin libexec pkglibexec pkgdata; do
  echo "${p}_SCRIPTS = $p.sh" >> Makefile.am
done

for p in data dataroot pkgdata doc html dvi pdf ps sysconf \
         sharedstate localstate lisp; do
  echo "${p}_DATA = $p.dat" >> Makefile.am
done

for p in include oldinclude pkginclude; do
  echo "${p}_HEADERS = $p.h" >> Makefile.am
done

for p in man man1 man2 man3 man4 man5 man6 man7 man8 man9; do
  echo "${p}_MANS = bar.$p"
done

echo "info_TEXINFOS = foo.texi" >> Makefile.am
echo "lisp_LISP = foo.el" >> Makefile.am
echo "python_PYTHON = foo.py" >> Makefile.am

awk '{print NR ":" $0}' Makefile.am # For debugging.

# Go with the tests.

$ACLOCAL
$AUTOMAKE

:
