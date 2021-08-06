#! /bin/sh
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

# Check that $(LFLAGS) takes precedence over both $(AM_LFLAGS) and
# $(foo_LFLAGS).
# Please keep this in sync with the sister tests:
#  - lflags-cxx.sh
#  - yflags.sh
#  - yflags-cxx.sh

required=cc
. test-init.sh

cat >fake-lex <<'END'
#!/bin/sh
echo '/*' "$*" '*/' >lex.yy.c
echo 'extern int dummy;' >> lex.yy.c
END
chmod a+x fake-lex

cat >> configure.ac <<'END'
AC_SUBST([CC], [false])
# Simulate presence of Lex using our fake-lex script.
AC_SUBST([LEX], ['$(abs_top_srcdir)'/fake-lex])
AC_SUBST([LEX_OUTPUT_ROOT], [lex.yy])
AC_SUBST([LEXLIB], [''])
AC_OUTPUT
END

cat > Makefile.am <<'END'
AUTOMAKE_OPTIONS = no-dependencies
bin_PROGRAMS = foo bar
foo_SOURCES = main.c foo.l
bar_SOURCES = main.c bar.l
AM_LFLAGS = __am_flags__
bar_LFLAGS = __bar_flags__
END

$ACLOCAL
$AUTOMAKE -a

grep '\$(LFLAGS).*\$(bar_LFLAGS)' Makefile.in && exit 1
grep '\$(LFLAGS).*\$(AM_LFLAGS)' Makefile.in && exit 1

: > foo.l
: > bar.l

$AUTOCONF
./configure
run_make LFLAGS=__user_flags__ foo.c bar-bar.c

cat foo.c
cat bar-bar.c

grep '__am_flags__.*__user_flags__' foo.c
grep '__bar_flags__.*__user_flags__' bar-bar.c

:
