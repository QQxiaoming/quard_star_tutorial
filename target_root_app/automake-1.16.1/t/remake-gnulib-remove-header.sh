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

# Test remake rules when a C header "guarded" by AC_SUBST'd variables
# is not needed anymore, or when it's needed again.
# This test requires some user-level machinery, overlaps with other tests,
# and is not strictly necessary per se, but it exercises a real, important
# use case (from gnulib, see:
#  <https://lists.gnu.org/archive/html/bug-gnulib/2011-04/msg00005.html>
# for more info).

required=cc
. test-init.sh

cat >> configure.ac <<'END'
AC_CONFIG_HEADERS([config.h])
AC_PROG_CC
MY_MACROS
AC_OUTPUT
END

cat > Makefile.am <<'END'
ACLOCAL_AMFLAGS = -I .
noinst_PROGRAMS = foo
foo_SOURCES = foo.c
BUILT_SOURCES = $(STDIO_H)
if REPLACE_STDIO_H
stdio.h: stdio.in.h $(top_builddir)/config.status
	cp $(srcdir)/stdio.in.h $@
else
stdio.h: $(top_builddir)/config.status
	rm -f $@
endif
MOSTLYCLEANFILES = stdio.h
END

cat > macros.m4 <<'END'
AC_DEFUN([MY_MACROS], [
  override_stdio=:
  if $override_stdio; then
    STDIO_H=stdio.h
    use_dummies=1
  else
    STDIO_H=
    use_dummies=0
  fi
  AC_SUBST([STDIO_H])
  AC_DEFINE_UNQUOTED([USE_DUMMIES], [$use_dummies],
    [Whether to use dummy types.])
  AM_CONDITIONAL([REPLACE_STDIO_H], [test -n "$STDIO_H"])
])
END

cat > stdio.in.h <<'END'
typedef struct dummyfile { void *p; } DUMMYFILE;
END

cat > foo.c <<'END'
#include <config.h>
#include <stdio.h>
#if USE_DUMMIES
DUMMYFILE *f;
#else
FILE *f;
#endif
int main () { return 0; }
END

$ACLOCAL -I .
$AUTOHEADER
$AUTOMAKE
$AUTOCONF

for vpath in : false; do

  if $vpath; then
    mkdir build
    cd build
    srcdir=..
  else
    srcdir=.
  fi

  # Do not reject slow dependency extractors: we need dependency tracking.
  $srcdir/configure --enable-dependency-tracking
  if $FGREP 'depmode=none' Makefile; then
    skip_ "automatic dependency tracking couldn't be activated"
  fi

  $MAKE
  ls -l
  test -f stdio.h

  # Simulate that we don't need our custom stdio.h anymore.

  $sleep
  sed -e 's/^\( *override_stdio\)=.*$/\1=false/' $srcdir/macros.m4 > t
  diff $srcdir/macros.m4 t && fatal_ "failed to edit macros.m4"
  mv -f t $srcdir/macros.m4

  using_gmake || $MAKE Makefile
  $MAKE
  ls -l
  test ! -e stdio.h

  # And now simulate that we want our custom stdio.h back.

  $sleep
  sed -e 's/^\( *override_stdio\)=.*$/\1=:/' $srcdir/macros.m4 > t
  diff $srcdir/macros.m4 t && fatal_ "failed to edit macros.m4"
  mv -f t $srcdir/macros.m4

  using_gmake || $MAKE Makefile
  $MAKE
  ls -l
  test -f stdio.h

  $MAKE distclean
  cd $srcdir

done

:
