#!/bin/sh
# Copyright (C) 2009-2018 Free Software Foundation, Inc.
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

# Check silent-rules mode, with many languages at once.
# This test partly overlaps with other 'silent*.sh', but it serves as
# a stress test by using many different languages at once -- so don't
# remove this test script.

required='cc c++ fortran fortran77 lex yacc'
. test-init.sh

# Avoids too much code duplication.
do_and_check_silent_build ()
{
  case $1 in
    --rebuild) rebuild=true;;
            *) rebuild=false;;
  esac

  run_make -O
  # Avoid spurious failures with SunStudio Fortran compilers.
  sed '/^NOTICE:/d' stdout > t
  mv -f t stdout
  cat stdout

  $EGREP ' (-c|-o)' stdout && exit 1
  $EGREP '(mv|ylwrap) ' stdout && exit 1

  grep 'CXX .*foo1\.' stdout
  grep 'CXX .*baz1\.' stdout
  grep 'FC .*foo2\.'  stdout
  grep 'FC .*baz2\.'  stdout
  grep 'F77 .*foo3\.' stdout
  grep 'F77 .*baz3\.' stdout
  grep ' CC .*foo5\.' stdout
  grep ' CC .*baz5\.' stdout
  grep ' CC .*foo6\.' stdout
  grep ' CC .*baz6\.' stdout

  grep 'CXXLD .*foo' stdout
  grep 'CCLD .*bar'  stdout
  grep 'CXXLD .*baz' stdout
  grep 'CCLD .*bla'  stdout

  if ! $rebuild; then
    grep 'YACC .*foo6\.' stdout
    grep 'YACC .*baz6\.' stdout
    grep 'LEX .*foo5\.'  stdout
    grep 'LEX .*baz5\.'  stdout
  fi

  unset rebuild
}

# Avoids too much code duplication.
do_and_check_verbose_build ()
{
  case $1 in
    --rebuild) rebuild=true;;
            *) rebuild=false;;
  esac

  run_make -O V=1

  grep ' -c ' stdout
  grep ' -o ' stdout

  $EGREP '(CC|CXX|FC|F77|LD) ' stdout && exit 1

  if ! $rebuild; then
    grep 'ylwrap ' stdout
    $EGREP '(LEX|YACC) ' stdout && exit 1
  fi

  unset rebuild
}

mkdir sub

cat >>configure.ac <<'EOF'
AC_PROG_F77
AC_PROG_FC
AC_PROG_LEX
AC_PROG_YACC
AC_PROG_CXX

# The SunStudio C++ compiler is unfortunately named 'sunCC' (or even just
# 'CC', yuck!); similarly and the Portland group C++ compiler is named
# 'pgCC'.  This can cause problems with our grepping checks on the output
# from make.  Avoid these problems by invoking a wrapper script, as
# filtering the make output proved too fragile.
case " $CXX " in
  *'CC '*)
    AC_MSG_WARN([the C++ compiler '$CXX' name ends with 'CC'])
    AC_MSG_WARN([it will be wrapped with the custom script 'am--cxx'])
    echo '#!/bin/sh' > bin/am--cxx
    echo 'PATH=$saved_PATH; export PATH' >> bin/am--cxx
    echo "case \$# in" >> bin/am--cxx
    echo "  0) exec $CXX ;;" >> bin/am--cxx
    echo "  *) exec $CXX \"\$@\" ;;" >> bin/am--cxx
    echo "esac" >> bin/am--cxx
    chmod a+x bin/am--cxx
    CXX=am--cxx
esac

AC_CONFIG_FILES([sub/Makefile])
AC_OUTPUT
EOF

cat > Makefile.am <<'EOF'
# Need generic and non-generic rules.
bin_PROGRAMS = foo bar fo2
bar_CFLAGS = $(AM_CFLAGS)
foo_SOURCES = foo1.cpp foo2.f90 foo3.f foo5.l foo6.y
fo2_SOURCES = $(foo_SOURCES)
fo2_CPPFLAGS = $(AM_CPPFLAGS)
fo2_FFLAGS = $(AM_FFLAGS)
fo2_FCFLAGS = $(AM_FCFLAGS)
fo2_YFLAGS = -v
fo2_LFLAGS = -n
fo2_LDADD = $(LEXLIB)
SUBDIRS = sub
AM_YFLAGS = -d
BUILT_SOURCES = foo6.h
EOF

cat > sub/Makefile.am <<'EOF'
AUTOMAKE_OPTIONS = subdir-objects
# Need generic and non-generic rules.
bin_PROGRAMS = baz bla ba2
bla_CFLAGS = $(AM_CFLAGS)
baz_SOURCES = baz1.cpp baz2.f90 baz3.f baz5.l baz6.y
ba2_SOURCES = $(baz_SOURCES)
ba2_CPPFLAGS = $(AM_CPPFLAGS)
ba2_FFLAGS = $(AM_FFLAGS)
ba2_FCFLAGS = $(AM_FCFLAGS)
ba2_YFLAGS = -v
ba2_LFLAGS = -n
ba2_LDADD = $(LEXLIB)
AM_YFLAGS = -d
BUILT_SOURCES = baz6.h
EOF

cat > foo1.cpp <<'EOF'
int main ()
{
  return 0;
}
EOF
cat > foo2.f90 <<'EOF'
      subroutine foo2
      return
      end
EOF
cat > foo3.f <<'EOF'
      subroutine foo3
      return
      end
EOF
cat > foo5.l <<'EOF'
%{
#define YY_NO_UNISTD_H 1
int isatty (int fd) { return 0; }
%}
%%
"END"   return EOF;
.
%%
/* Avoid possible link errors. */
int yywrap (void)
{
  return 1;
}
EOF
cat > foo6.y <<'EOF'
%{
extern int yylex (void);
void yyerror (char *s) {}
%}
%token EOF
%%
fubar : 'f' 'o' 'o' 'b' 'a' 'r' EOF {};
EOF
cp foo1.cpp bar.c
cp foo1.cpp sub/baz.c
cp foo1.cpp sub/bla.c
cp foo1.cpp sub/baz1.cpp
cp foo2.f90 sub/baz2.f90
cp foo3.f sub/baz3.f
cp foo5.l sub/baz5.l
cp foo6.y sub/baz6.y

mkdir bin
saved_PATH=$PATH; export saved_PATH
PATH=$(pwd)/bin$PATH_SEPARATOR$PATH; export PATH

$ACLOCAL
$AUTOMAKE --add-missing
$AUTOCONF

# Ensure per-target rules are used, to ensure their coverage below.
# (We do not do an exhaustive check, that wouldn't be practical).
$FGREP 'bar-bar.o' Makefile.in
$FGREP 'fo2-foo5.c' Makefile.in
$FGREP 'fo2-foo6.c' Makefile.in

# Force dependency tracking explicitly, so that slow dependency
# extractors are not rejected.  Try also with dependency tracking
# explicitly disabled.
for config_args in \
  --enable-dependency-tracking --disable-dependency-tracking
do

  ./configure $config_args --enable-silent-rules

  do_and_check_silent_build
  # Cleaning and then rebuilding with the same V flag (and without
  # removing the generated sources in between) shouldn't trigger a
  # different set of rules.
  $MAKE clean
  do_and_check_silent_build --rebuild

  # Ensure a clean rebuild.
  $MAKE clean
  # This is required, since these files are not removed by 'make clean'
  # (as dictated by the GNU Coding Standards).
  rm -f *foo5.c *foo6.[ch] sub/*baz5.c sub/*baz6.[ch]

  do_and_check_verbose_build
  # Cleaning and then rebuilding with the same V flag (and without
  # removing the generated sources in between) shouldn't trigger a
  # different set of rules.
  $MAKE clean
  do_and_check_verbose_build --rebuild

  # Ensure a clean reconfiguration/rebuild.
  $MAKE clean
  $MAKE maintainer-clean

done

:
