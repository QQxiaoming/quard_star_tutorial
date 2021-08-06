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

# Check silent-rules mode for Lex.

required='cc lex'
. test-init.sh

mkdir sub

cat >>configure.ac <<'EOF'
AC_PROG_LEX
AC_CONFIG_FILES([sub/Makefile])
AC_OUTPUT
EOF

cat > Makefile.am <<'EOF'
# Need generic and non-generic rules.
bin_PROGRAMS = foo1 foo2
foo1_SOURCES = foo.l
foo2_SOURCES = $(foo1_SOURCES)
foo2_LFLAGS = -n
foo2_CFLAGS = $(AM_CFLAGS)
SUBDIRS = sub
LDADD = $(LEXLIB)
EOF

cat > sub/Makefile.am <<'EOF'
AUTOMAKE_OPTIONS = subdir-objects
# Need generic and non-generic rules.
bin_PROGRAMS = bar1 bar2
bar1_SOURCES = bar.l
bar2_SOURCES = $(bar1_SOURCES)
bar2_LFLAGS = -n
bar2_CFLAGS = $(AM_CFLAGS)
LDADD = $(LEXLIB)
EOF

cat > foo.l <<'EOF'
%{
#define YY_NO_UNISTD_H 1
%}
%%
"END"   return EOF;
.
%%
/* Avoid possible link errors. */
int yywrap (void) { return 1; }
int   main (void) { return 0; }
EOF
cp foo.l sub/bar.l

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

grep 'LEX .*foo\.' stdout
grep 'LEX .*bar\.' stdout
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

# Don't look for LEX, as probably lex hasn't been re-run.
grep ' CC .*foo\.' stdout
grep ' CC .*bar\.' stdout
grep 'CCLD .*foo1' stdout
grep 'CCLD .*bar1' stdout
grep 'CCLD .*foo2' stdout
grep 'CCLD .*bar2' stdout

# Ensure a truly clean rebuild.
$MAKE clean
rm -f *foo.c sub/*bar.c

run_make -O V=1

grep ' -c ' stdout
grep ' -o ' stdout
grep 'ylwrap ' stdout

$EGREP '(LEX|CC|CCLD) ' stdout && exit 1

# Cleaning and then rebuilding with the same V flag (and without
# removing the generated sources in between) shouldn't trigger a
# different set of rules.
$MAKE clean

run_make -O V=1

# Don't look for ylwrap, as probably lex hasn't been re-run.
grep ' -c ' stdout
grep ' -o ' stdout

$EGREP '(LEX|CC|CCLD) ' stdout && exit 1

:
