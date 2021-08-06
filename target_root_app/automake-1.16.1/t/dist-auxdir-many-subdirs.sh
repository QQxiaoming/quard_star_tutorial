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

# It should be legitimate for many subdir Makefiles to require the
# same config-aux files.

am_create_testdir=empty
required=cc
. test-init.sh

count=0
ocwd=$(pwd) || fatal_ "cannot get current working directory"

# Usage: do_check [--add-missing] [CONFIG-AUXDIR-PATH=.]
do_check ()
{
  case $1 in
    -a|--add-missing) add_missing=yes; shift;;
    *) add_missing=no;;
  esac
  auxdir=${1-.}

  count=$(($count + 1))
  mkdir T$count.d
  cd T$count.d

  distdir=$me-$count
  unindent > configure.ac << END
    AC_INIT([$me], [$count])
    AC_CONFIG_AUX_DIR([$auxdir])
    AM_INIT_AUTOMAKE
    AC_PROG_CC
    # We don't want to require python or emcas in this test, so
    # the tricks below.
    AM_PATH_PYTHON([2.2], [], [:])
    EMACS=no; AM_PATH_LISPDIR
    AC_CONFIG_FILES([Makefile])
END

  unindent > Makefile.stub <<'END'
    ## For depcomp.
    bin_PROGRAMS = foo
    foo_SOURCES = foo.c
    ## For py-compile.
    python_PYTHON = bar.py
    ## For test-driver.
    TESTS =
END

  required_files='
    install-sh
    missing
    compile
    depcomp
    py-compile
    test-driver
  '

  echo "SUBDIRS =" > Makefile.am

  suffixes='0 1 2 3 4 5 6 7 8 9'

  for x in $suffixes; do
    mkdir sub$x
    echo "SUBDIRS += sub$x" >> Makefile.am
    echo "AC_CONFIG_FILES([sub$x/Makefile])" >> configure.ac
    cp Makefile.stub sub$x/Makefile.am
    echo 'int main (void) { return 0; }' > sub$x/foo.c
    touch sub$x/bar.py sub$x/baz.el
  done
  echo AC_OUTPUT >> configure.ac

  $ACLOCAL
  $AUTOCONF

  "$am_scriptdir"/install-sh -d $auxdir \
    || fatal_ "creating directory '$auxdir' with install-sh"
  if test $add_missing = yes; then
    $AUTOMAKE -a --copy
    for f in $required_files; do
      test -f $auxdir/$f
      # To ensure that if a auxiliary file is required and distributed
      # by many Makefiles, the "dist" rule won't try to copy it multiple
      # times in $(distdir).
      chmod a-w $auxdir/$f
    done
  else
    for f in $required_files; do
      cp "$am_scriptdir"/$f $auxdir/$f \
        || fatal_ "faild to fetch auxiliary script '$f'"
      # See comments above.
      chmod a-w $auxdir/$f
    done
    $AUTOMAKE
  fi

  for vpath in : false; do
    if $vpath; then
      mkdir build
      cd build
      srcdir=..
    else
      srcdir=.
    fi
    $srcdir/configure
    $MAKE distdir
    find $distdir # For debugging.
    for f in $required_files; do
      test -f $distdir/$auxdir/$f
    done
    cd $srcdir
  done

  cd "$ocwd" || fatal_ "cannot chdir back to '$ocwd'"
}

do_check .
do_check --add-missing .
do_check build-aux
do_check --add-missing build-aux
do_check a/b/c
do_check --add-missing a/b/c

:
