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

# Test remake rules when a new C header "guarded" by AC_SUBST'd
# variables is added.
# This test overlaps with others, and is not strictly necessary per se,
# but it exercises a real use case (from gnulib, see:
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
stdio.h: stdio.in.h
	cp $(srcdir)/stdio.in.h $@
MOSTLYCLEANFILES = stdio.h
EXTRA_DIST = stdio.in.h
check-local:
	ls -l . $(srcdir)
	if test -n '$(STDIO_H)'; then \
	    test -f stdio.h || exit 1; \
	else \
	    test ! -f stdio.h || exit 1; \
	fi
END

cat > macros.m4 <<'END'
AC_DEFUN([MY_MACROS], [
  override_stdio=false
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

./configure

$MAKE
ls -l
test ! -e stdio.h
# Also try our build rules in a VPATH build.
$MAKE distcheck

# No need to sleep here: "./configure" and "make distcheck" above
# have already slept enough.

sed -e 's/^\( *override_stdio\)=.*$/\1=:/' macros.m4 > t
mv -f t macros.m4

using_gmake || $MAKE Makefile
$MAKE
ls -l
test -f stdio.h
# Also try our build rules in a VPATH build.
$MAKE distcheck

:
