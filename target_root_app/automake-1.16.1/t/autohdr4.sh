#!/bin/sh
# Copyright (C) 2003-2018 Free Software Foundation, Inc.
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

# Check rebuild rules for AC_CONFIG_HEADERS.
# (This should also work without GNU Make.)

required=cc
. test-init.sh

cat >>configure.ac <<'EOF'
AC_PROG_CC
AC_SUBST([BOT], [bot])
AC_CONFIG_HEADERS([defs.h config.h:sub1/config.top:sub2/config.${BOT}],,
                  [BOT=$BOT])
AC_CONFIG_FILES([sub3/Makefile])
AC_OUTPUT
EOF

mkdir sub1 sub2 sub3

: > sub1/config.top
echo '#define NAME "grepme1"' >sub2/config.bot

cat > Makefile.am <<'END'
SUBDIRS = sub3
.PHONY: test-prog-updated
test-prog-updated:
	is_newest sub3/run$(EXEEXT) sub2/config.bot
END

cat > sub3/Makefile.am <<'END'
noinst_PROGRAMS = run
END

cat >sub3/run.c <<'EOF'
#include <defs.h>
#include <config.h>
#include <stdio.h>

int main (void)
{
  puts (NAME); /* from config.h */
  puts (PACKAGE); /* from defs.h */
}
EOF


$ACLOCAL
$AUTOCONF
$AUTOHEADER
$AUTOMAKE

# Do not reject slow dependency extractors: we need dependency tracking.
./configure --enable-dependency-tracking
$MAKE
# Sanity check.
cross_compiling || sub3/run | grep grepme1 || exit 1

$sleep
echo '#define NAME "grepme2"' > sub2/config.bot
$MAKE
cross_compiling || sub3/run | grep grepme2 || exit 1
$MAKE test-prog-updated

$MAKE distcheck

:
