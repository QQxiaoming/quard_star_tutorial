#! /bin/sh
# Copyright (C) 2001-2018 Free Software Foundation, Inc.
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

# Check that automake yacc support ensures that yacc-generated C
# files use correct "#line" directives.  Try also with the
# 'subdir-object' option enabled.
# See also sister test 'lex-line.sh'.

required='cc yacc'
. test-init.sh

cat >> configure.ac << 'END'
AC_CONFIG_FILES([sub/Makefile])
AC_PROG_CC
AC_PROG_YACC
AC_OUTPUT
END

mkdir dir sub sub/dir

cat > Makefile.am << 'END'
SUBDIRS = sub
bin_PROGRAMS = foo bar
AM_YFLAGS = -d
bar_YFLAGS =
foo_SOURCES = zardoz.y
bar_SOURCES = dir/quux.y
## Avoid spurious failures with Solaris make.
zardoz.@OBJEXT@: zardoz.c
bar-quux.@OBJEXT@: bar-quux.c
END

cat > sub/Makefile.am << 'END'
AUTOMAKE_OPTIONS = subdir-objects
noinst_PROGRAMS = foo bar
foo_YFLAGS = -d
foo_SOURCES = zardoz.y
bar_SOURCES = dir/quux.y
## Avoid spurious failures with Solaris make.
foo-zardoz.@OBJEXT@: foo-zardoz.c
dir/quux.@OBJEXT@: dir/quux.c
END

cat > zardoz.y << 'END'
%{
int yylex () { return 0; }
void yyerror (char *s) { return; }
%}
%%
x : 'x' {};
%%
int main(void)
{
  return yyparse ();
}
END

cp zardoz.y dir/quux.y
cp zardoz.y sub/zardoz.y
cp zardoz.y sub/dir/quux.y

c_outputs='zardoz.c bar-quux.c sub/foo-zardoz.c sub/dir/quux.c'

$ACLOCAL
$AUTOCONF
# FIXME: stop disabling the warnings in the 'unsupported' category
# FIXME: once the 'subdir-objects' option has been mandatory.
$AUTOMAKE -a -Wno-unsupported

for vpath in : false; do

  if $vpath; then
    srcdir=..
    mkdir build
    cd build
  else
    srcdir=.
  fi

  $srcdir/configure
  $MAKE

  # For debugging,
  ls -l . sub sub/dir
  $EGREP 'line|\.y' $c_outputs

  # Adjusted "#line" should not contain reference to the builddir.
  grep '#.*line.*build.*\.y' $c_outputs && exit 1
  # Adjusted "#line" should not contain reference to the absolute
  # srcdir.
  $EGREP '#.*line *"?/.*\.y' $c_outputs && exit 1
  # Adjusted "#line" should not contain reference to the default
  # output file names, e.g., 'y.tab.c' and 'y.tab.h'.
  grep '#.*line.*y\.tab\.' $c_outputs && exit 1
  # Look out for a silly regression.
  grep "#.*\.y.*\.y" $c_outputs && exit 1
  if $vpath; then
    grep '#.*line.*"\.\./zardoz\.y"' zardoz.c
    grep '#.*line.*"\.\./dir/quux\.y"' bar-quux.c
    grep '#.*line.*"\.\./\.\./sub/zardoz\.y"' sub/foo-zardoz.c
    grep '#.*line.*"\.\./\.\./sub/dir/quux\.y"' sub/dir/quux.c
  else
    grep '#.*line.*"zardoz\.y"' zardoz.c
    grep '#.*line.*"dir/quux\.y"' bar-quux.c
    grep '#.*line.*"zardoz\.y"' sub/foo-zardoz.c
    grep '#.*line.*"dir/quux\.y"' sub/dir/quux.c
  fi

  cd $srcdir

done

:
