#!/bin/sh
# Copyright (C) 2010-2018 Free Software Foundation, Inc.
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

# Check silent-rules mode for Yacc.

required='cc yacc'
. test-init.sh

mkdir sub

cat >>configure.ac <<'EOF'
AC_PROG_CC
AC_PROG_YACC
AC_CONFIG_FILES([sub/Makefile])
AC_OUTPUT
EOF

cat > Makefile.am <<'EOF'
# Need generic and non-generic rules.
bin_PROGRAMS = foo1 foo2
foo1_SOURCES = foo.y
foo2_SOURCES = $(foo1_SOURCES)
foo2_YFLAGS = -v
foo2_CFLAGS = $(AM_CPPFLAGS)
SUBDIRS = sub
EOF

cat > sub/Makefile.am <<'EOF'
AUTOMAKE_OPTIONS = subdir-objects
# Need generic and non-generic rules.
bin_PROGRAMS = bar1 bar2
bar1_SOURCES = bar.y
bar2_SOURCES = $(bar1_SOURCES)
bar2_YFLAGS = -v
bar2_CFLAGS = $(AM_CPPFLAGS)
EOF

cat > foo.y <<'EOF'
%{
void yyerror (char *s) { return; }
int yylex (void) { return 0; }
int main (void) { return 0; }
%}
%token EOF
%%
fubar : 'f' 'o' 'o' 'b' 'a' 'r' EOF {};
EOF
cp foo.y sub/bar.y

$ACLOCAL
$AUTOMAKE --add-missing
$AUTOCONF

# Ensure per-target rules are used, to ensure their coverage below.
$FGREP 'foo2-foo.c' Makefile.in || exit 99
$FGREP 'bar2-bar.c' sub/Makefile.in || exit 99

./configure --enable-silent-rules

run_make -O

$EGREP ' (-c|-o)' stdout && exit 1
$EGREP '(mv|ylwrap) ' stdout && exit 1

grep 'YACC .*foo\.' stdout
grep 'YACC .*bar\.' stdout
grep ' CC .*foo\.' stdout
grep ' CC .*bar\.' stdout
grep 'CCLD .*foo1' stdout
grep 'CCLD .*bar1' stdout
grep 'CCLD .*foo2' stdout
grep 'CCLD .*bar2' stdout

# Cleaning and then rebuilding with the same V flag (and without
# removing the generated sources in between) shouldn't trigger a
# different set of rules.
$MAKE clean

run_make -O

$EGREP ' (-c|-o)' stdout && exit 1
$EGREP '(mv|ylwrap) ' stdout && exit 1

# Don't look for YACC, as probably yacc hasn't been re-run.
grep ' CC .*foo\.' stdout
grep ' CC .*bar\.' stdout
grep 'CCLD .*foo1' stdout
grep 'CCLD .*bar1' stdout
grep 'CCLD .*foo2' stdout
grep 'CCLD .*bar2' stdout

# Ensure a truly clean rebuild.
$MAKE clean
rm -f *foo.[ch] sub/*bar.[ch]

run_make -O V=1

grep ' -c ' stdout
grep ' -o ' stdout
grep 'ylwrap ' stdout

$EGREP '(YACC|CC|CCLD) ' stdout && exit 1

# Cleaning and then rebuilding with the same V flag (and without
# removing the generated sources in between) shouldn't trigger a
# different set of rules.
$MAKE clean

run_make -O V=1

# Don't look for ylwrap, as probably lex hasn't been re-run.
grep ' -c ' stdout
grep ' -o ' stdout

$EGREP '(YACC|CC|CCLD) ' stdout && exit 1

:
