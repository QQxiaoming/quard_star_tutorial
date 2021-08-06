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

# An empty "foo_PRIMARY" declaration should *not* cause "make install"
# to create directory $(foodir).  See automake bug#10997 and bug#11030.

. test-init.sh

cat >> configure.ac <<'END'
AC_SUBST([CC], [whatever])
AC_SUBST([JAVAC], [whatever])
AM_PATH_PYTHON(,,:)
AC_OUTPUT
END

cat > Makefile.am << 'END'
AUTOMAKE_OPTIONS = no-dependencies
mydir = $(prefix)/my
myexecdir = $(prefix)/myexec
javadir = $(prefix)/java
END

# Write Makefile.am.
{

  for dir in sbin bin libexec pkglibexec myexec; do
    for dst in '' dist_ nodist_; do
      echo "${dst}${dir}_SCRIPTS ="
    done
    echo "${dir}_PROGRAMS ="
  done

  for dir in lib pkglib myexec my; do
    echo "${dir}_LIBRARIES ="
    echo "${dir}_LTLIBRARIES ="
  done

  for p1 in '' notrans_; do
    for p2 in '' dist_ nodist_; do
      for s in '' 1 2 3 4 5 6 7 8 9; do
        echo "${p1}${p2}man${s}_MANS ="
      done
    done
  done

  for dst in '' dist_ nodist_; do
    for dir in dataroot data pkgdata doc lisp my; do
      echo "${dst}${dir}_DATA ="
    done
    for dir in include pkginclude oldinclude my; do
      echo "${dst}${dir}_HEADERS ="
    done
    for dir in python my; do
      echo "${dst}${dir}_PYTHON ="
    done
    for dir in info my; do
      echo "${dst}${dir}_TEXINFOS ="
    done
    for dir in java my; do
      echo "${dst}${dir}_JAVA ="
    done
    for dir in lisp my; do
      echo "${dst}${dir}_LISP ="
    done
  done

} > t

cat t >> Makefile.am
sed 's/^/nobase_/' t >> Makefile.am
rm -f t

cat Makefile.am # For debugging.

# Sanity check.
grep '^oldinclude_HEADERS =' Makefile.am \
  && grep '^nodist_my_PYTHON =' Makefile.am \
  && grep '^notrans_dist_man5_MANS =' Makefile.am \
  || fatal_ "creating Makefile.am"

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

cwd=$(pwd) || fatal_ "getting current working directory"

doinst ()
{
  run_make install install-pdf install-ps install-dvi ${1+"$@"}
}

: > foo.sh

./configure --prefix="$cwd/inst"
doinst
test ! -e inst || { find inst; exit 1; }
$MAKE uninstall
doinst bin_SCRIPTS=foo.sh
test -f inst/bin/foo.sh

# Explicitly pass prefix to avoid spurious influences from
# global config.site scripts.
./configure --prefix="/usr/local"
test ! -e dest || { find dest; exit 1; }
$MAKE uninstall
doinst DESTDIR="$cwd/dest" bin_SCRIPTS=foo.sh
test -f dest/usr/local/bin/foo.sh

:
