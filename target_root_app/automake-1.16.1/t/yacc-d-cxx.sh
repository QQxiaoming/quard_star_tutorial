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

# Various tests on Yacc/C++ support with yacc-generated headers
# (i.e., '-d' in *YFLAGS).
# Keep in sync with sister test 'yacc-d-basic.sh'.

required='c++ yacc'
. test-init.sh

write_parse ()
{
  header=$1
  unindent <<END
    %{
    // Valid C++, but deliberately invalid C.
    #include <cstdlib>
    #include "$header"
    int yylex (void) { return 0; }
    void yyerror (const char *s) {}
    %}
    %%
    x : 'x' {};
    %%
END
}

write_main ()
{
  header=$1
  unindent <<END
    // Valid C++, but deliberately invalid C.
    #include <cstdio>
    #include "$header"
    int main (int argc, char **argv)
    {
      int yyparse (void);
      return yyparse ();
    }
END
}

cat >> configure.ac << 'END'
AC_PROG_CXX
AC_PROG_YACC
AC_CONFIG_FILES([foo/Makefile bar/Makefile baz/Makefile qux/Makefile])
AC_OUTPUT
END

mkdir foo bar baz qux baz/sub

# These makefiles will be extended later.
cat > Makefile.am <<'END'
.PHONY: echo-distcom
echo-distcom:
	@echo ' ' $(DIST_COMMON) ' '
END
cp Makefile.am foo/Makefile.am
cp Makefile.am bar/Makefile.am
cp Makefile.am baz/Makefile.am
cp Makefile.am qux/Makefile.am

cat >> Makefile.am <<'END'
SUBDIRS = foo bar baz qux
END

$ACLOCAL
$AUTOCONF

cp "$am_scriptdir/ylwrap" . \
  || fatal_ "cannot fetch auxiliary script 'ylwrap'"

$AUTOMAKE Makefile

# Try with -d in $(YFLAGS) (don't do this in real life!).
cat >> foo/Makefile.am <<END
bin_PROGRAMS = zardoz
zardoz_SOURCES = parse.yy main.cc
BUILT_SOURCES = parse.hh
YFLAGS=\
-d
END

$AUTOMAKE -Wno-gnu foo/Makefile

write_parse parse.hh > foo/parse.yy
write_main parse.hh > foo/main.cc

# Try with -d in $(AM_YFLAGS).
cat >> bar/Makefile.am <<END
bin_PROGRAMS = zardoz
zardoz_SOURCES = parse.ypp main.cpp
BUILT_SOURCES = parse.hpp
AM_YFLAGS${tab}=  -d ${tab}
END

$AUTOMAKE bar/Makefile

write_parse parse.hpp > bar/parse.ypp
write_main parse.hpp > bar/main.cpp

# Try with -d in $(AM_YFLAGS), and a subdir parser.
cat >> baz/Makefile.am <<END
AUTOMAKE_OPTIONS = subdir-objects
bin_PROGRAMS = joe
joe_SOURCES = sub/parse.y++ sub/main.c++
BUILT_SOURCES = sub/parse.h++
AM_YFLAGS = \
${tab}-d
END

$AUTOMAKE baz/Makefile

write_parse sub/parse.h++ > baz/sub/parse.y++
write_main sub/parse.h++ > baz/sub/main.c++

# Try with -d in $(xxx_YFLAGS) (per-object flag).
cat >> qux/Makefile.am <<END
bin_PROGRAMS = maude
maude_SOURCES = parse.yxx main.cxx
maude_YFLAGS=${tab}  -d${tab}
BUILT_SOURCES = maude-parse.hxx
END

$AUTOMAKE qux/Makefile

write_parse maude-parse.hxx > qux/parse.yxx
write_main maude-parse.hxx > qux/main.cxx

./configure

$MAKE
ls -l . foo bar baz baz/sub qux # For debugging.

test -f foo/parse.cc
test -f foo/parse.hh
test -f bar/parse.cpp
test -f bar/parse.hpp
test -f baz/sub/parse.c++
test -f baz/sub/parse.h++
test -f qux/maude-parse.cxx
test -f qux/maude-parse.hxx

# The ylwrap script must be shipped.
$MAKE echo-distcom
$MAKE -s echo-distcom | grep '[ /]ylwrap '

# The generated C++ source and header files must be shipped.
cd foo
$MAKE echo-distcom
$MAKE -s echo-distcom | grep '[ /]parse\.cc '
$MAKE -s echo-distcom | grep '[ /]parse\.hh '
cd ..
cd bar
$MAKE echo-distcom
$MAKE -s echo-distcom | grep '[ /]parse\.cpp '
$MAKE -s echo-distcom | grep '[ /]parse\.hpp '
cd ..
cd baz
$MAKE echo-distcom
$MAKE -s echo-distcom | grep '[ /]sub/parse\.c++ '
$MAKE -s echo-distcom | grep '[ /]sub/parse\.h++ '
cd ..
cd qux
$MAKE echo-distcom
$MAKE -s echo-distcom | grep '[ /]maude-parse\.cxx '
$MAKE -s echo-distcom | grep '[ /]maude-parse\.hxx '
cd ..

$MAKE distdir
find $distdir # For debugging.

test -f $distdir/ylwrap
test -f $distdir/foo/parse.cc
test -f $distdir/foo/parse.hh
test -f $distdir/bar/parse.cpp
test -f $distdir/bar/parse.hpp
test -f $distdir/baz/sub/parse.c++
test -f $distdir/baz/sub/parse.h++
test -f $distdir/qux/maude-parse.cxx
test -f $distdir/qux/maude-parse.hxx

# The Yacc-derived C++ sources must be created, and not removed once
# compiled (i.e., not treated like "intermediate files" in the GNU
# make sense).
yl_distcheck

# Check that we can recover from deleted headers.
$MAKE clean
rm -f foo/parse.hh bar/parse.hpp baz/sub/parse.h++ qux/maude-parse.hxx
$MAKE
test -f foo/parse.hh
test -f bar/parse.hpp
test -f baz/sub/parse.h++
test -f qux/maude-parse.hxx

# Make sure that the Yacc-derived C++ sources are erased by
# maintainer-clean, and not by distclean.
$MAKE distclean
test -f foo/parse.cc
test -f foo/parse.hh
test -f bar/parse.cpp
test -f bar/parse.hpp
test -f baz/sub/parse.c++
test -f baz/sub/parse.h++
test -f qux/maude-parse.cxx
test -f qux/maude-parse.hxx
./configure # Re-create 'Makefile'.
$MAKE maintainer-clean
test ! -e foo/parse.cc
test ! -e foo/parse.hh
test ! -e bar/parse.cpp
test ! -e bar/parse.hpp
test ! -e baz/sub/parse.c++
test ! -e baz/sub/parse.h++
test ! -e qux/maude-parse.cxx
test ! -e qux/maude-parse.hxx

:
