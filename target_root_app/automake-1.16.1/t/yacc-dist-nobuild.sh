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

# Check that distributed Yacc-generated parsers are not uselessly
# remade from an unpacked distributed tarball.
# See automake bug#7884.

required='cc yacc'
. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_CC
AC_PROG_YACC
AC_OUTPUT
END

cat > Makefile.am << 'END'
bin_PROGRAMS = foobar zardoz
foobar_SOURCES = parse.y main.c
zardoz_SOURCES = $(foobar_SOURCES)
zardoz_YFLAGS = -d
END

cat > parse.y << 'END'
%{
int yylex () { return 0; }
void yyerror (char *s) {}
%}
%%
foobar : 'f' 'o' 'o' 'b' 'a' 'r' {};
END

cat > main.c << 'END'
int main () { return 0; }
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure
$MAKE

$MAKE distdir
chmod -R a-w $distdir

mkdir bin
cat > bin/yacc <<'END'
#!/bin/sh
echo "$0 invoked, shouldn't happen!" >&2
exit 1
END
cp bin/yacc bin/bison
chmod a+x bin/yacc bin/bison
PATH=$(pwd)/bin$PATH_SEPARATOR$PATH

YACC=yacc BISON=bison
export YACC BISON

mkdir build
cd build
../$distdir/configure
$MAKE

# Sanity check.
cd ..
chmod u+w $distdir
rm -f $distdir/parse.c
chmod a-w $distdir
mkdir build2
cd build2
../$distdir/configure
run_make -e FAIL -M
$FGREP parse.c output

:
