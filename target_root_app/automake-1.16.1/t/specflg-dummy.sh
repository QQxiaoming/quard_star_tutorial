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

# Check that even "dummy" per-target flags triggers the use of renamed
# objects.  I.e., a definition like 'foo_CFLAGS = $(AM_CFLAGS)' should
# always cause Automake to trigger the semantics for per-target CFLAGS,
# even if AM_CFLAGS is undefined. Similarly for other *FLAGS variables
# (CXXFLAGS, YFLAGS, LDFLAGS, ...)

. test-init.sh

# Disable shell globbing if possible.
(set +f) >/dev/null 2>&1 && set +f

oIFS=$IFS
nl='
'

matches=
add_match ()
{
  matches="$matches$nl$1"
}

do_check ()
{
  IFS=$nl
  for string in $matches; do
    IFS=$oIFS
    $FGREP "$string" Makefile.in
  done
  IFS=$oIFS
}

# Fake libtool presence, so that we won't have to require it.
cat > acinclude.m4 <<END
AC_DEFUN([AC_PROG_LIBTOOL], [AC_SUBST([LIBTOOL], [dummy])])
END
: > ltmain.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AC_PROG_CXX
AC_PROG_YACC
AC_PROG_LEX
AC_PROG_F77
AC_PROG_FC
AM_PROG_GCJ
AM_PROG_AR
AC_PROG_RANLIB
AC_PROG_LIBTOOL
AM_PROG_UPC
AC_PROG_OBJC
END

cat > Makefile.am <<'END'
bin_PROGRAMS =
lib_LIBRARIES =
lib_LTLIBRARIES =
END

$ACLOCAL

cat >> Makefile.am <<'END'
bin_PROGRAMS += prog1
prog1_SOURCES = source1.c
prog1_CFLAGS = $(AM_CFLAGS)
END

add_match 'prog1-source1.$(OBJEXT)'

cat >> Makefile.am <<'END'
bin_PROGRAMS += prog2
prog2_SOURCES = source2.c
prog2_CPPFLAGS = $(AM_CPPFLAGS)
END

add_match 'prog2-source2.$(OBJEXT)'

cat >> Makefile.am <<'END'
bin_PROGRAMS += prog3
prog3_SOURCES = source3.cxx
prog3_CXXFLAGS = $(AM_CXXFLAGS)
END

add_match 'prog3-source3.$(OBJEXT)'

cat >> Makefile.am <<'END'
bin_PROGRAMS += prog4
prog4_SOURCES = source4.c++
prog4_CPPFLAGS = $(AM_CPPFLAGS)
END

add_match 'prog4-source4.$(OBJEXT)'

cat >> Makefile.am <<'END'
bin_PROGRAMS += prog5
prog5_SOURCES = source5.f
prog5_FFLAGS = $(AM_FFLAGS)
END

add_match 'prog5-source5.$(OBJEXT)'

cat >> Makefile.am <<'END'
bin_PROGRAMS += prog6
prog6_SOURCES = source6.f90
prog6_FCFLAGS = $(AM_FCFLAGS)
END

add_match 'prog6-source6.$(OBJEXT)'

cat >> Makefile.am <<'END'
bin_PROGRAMS += prog7
prog7_SOURCES = source7.r
prog7_RFLAGS = $(AM_RFLAGS)
END

add_match 'prog7-source7.$(OBJEXT)'

cat >> Makefile.am <<'END'
bin_PROGRAMS += prog8
prog8_SOURCES = source8.java
prog8_GCJFLAGS = $(AM_GCJFLAGS)
END

add_match 'prog8-source8.$(OBJEXT)'

cat >> Makefile.am <<'END'
bin_PROGRAMS += prog9
prog9_SOURCES = source9.upc
prog9_UPCFLAGS = $(AM_UPCFLAGS)
END

add_match 'prog9-source9.$(OBJEXT)'

cat >> Makefile.am <<'END'
bin_PROGRAMS += prog10
prog10_SOURCES = source10.m
prog10_OBJCFLAGS = $(AM_OBJCFLAGS)
END

add_match 'prog10-source10.$(OBJEXT)'

cat >> Makefile.am <<'END'
bin_PROGRAMS += linkprog
linkprog_SOURCES = linkprog.c
linkprog_LDFLAGS = $(AM_LDFLAGS)
END

add_match 'linkprog_LINK ='

cat >> Makefile.am <<'END'
lib_LIBRARIES += libstatic.a
libstatic_a_SOURCES = static123.c
libstatic_a_CFLAGS = $(AM_CFLAGS)
END

add_match 'libstatic_a-static123.$(OBJEXT)'

cat >> Makefile.am <<'END'
lib_LTLIBRARIES += libshared1.la
libshared1_la_SOURCES = shared1.c
libshared1_la_LIBTOOLFLAGS = $(AM_LIBTOOLFLAGS)
END

add_match 'libshared1_la-shared1.lo'

cat >> Makefile.am <<'END'
lib_LTLIBRARIES += libshared2.la
libshared2_la_SOURCES = shared2.cc
libshared2_la_CXXFLAGS = $(AM_CXXFLAGS)
END

add_match 'libshared2_la-shared2.lo'

cat >> Makefile.am <<'END'
bin_PROGRAMS += parse1
parse1_SOURCES = parse.y
parse1_YFLAGS = $(AM_YFLAGS)
END

add_match 'parse1-parse.c'

cat >> Makefile.am <<'END'
bin_PROGRAMS += parse2
parse2_SOURCES = parse.ypp
parse2_YFLAGS = $(AM_YFLAGS)
END

add_match 'parse2-parse.cpp'

cat >> Makefile.am <<'END'
bin_PROGRAMS += lexer1
lexer1_SOURCES = lex.l
lexer1_LFLAGS = $(AM_YFLAGS)
END

add_match 'lexer1-lex.c'

cat >> Makefile.am <<'END'
bin_PROGRAMS += lexer2
lexer2_SOURCES = lexer.ll
lexer2_LFLAGS = $(AM_YFLAGS)
END

add_match 'lexer2-lexer.cc'

# For debugging.
cat Makefile.am

$AUTOMAKE -a
do_check

sed '
  s|^\(.*\)_SOURCES *= *|sub_\1_SOURCES = srcsub/|
  s|^\(.*\)PROGRAMS *+= *|\1PROGRAMS += sub/|
  s|^\(.*\)LIBRARIES *+= *|\1LIBRARIES += sub/|
  s|^\(.*\)FLAGS *=|sub_\1FLAGS =|
' Makefile.am > t

cat - t > Makefile.am <<'END'
AUTOMAKE_OPTIONS = subdir-objects
END

rm -f t

# For debugging.
cat Makefile.am

$AUTOMAKE
do_check

:
