#! /bin/sh
# Copyright (C) 1999-2018 Free Software Foundation, Inc.
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

# Test of subdir objects with C and C++.

. test-init.sh

cat >> configure.ac <<'END'
AC_PROG_CC
AC_PROG_CXX
AC_PROG_YACC
AC_CONFIG_FILES([sub/Makefile])
AC_OUTPUT
END

$ACLOCAL
: > ylwrap

cat > Makefile.am << 'END'
SUBDIRS = sub
bin_PROGRAMS = wish
wish_SOURCES = generic/a.c
wish_SOURCES += another/z.cxx
END

mkdir sub
cat > sub/Makefile.am << 'END'
dream_SOURCES = generic/b.c more/r.y
bin_PROGRAMS = dream
END

AUTOMAKE_fails
grep "^Makefile\.am:3:.*'generic/a\.c'.* in a subdirectory" stderr
grep "^Makefile\.am:[34]:.*'another/z\.cxx'.* in a subdirectory" stderr
grep "^sub/Makefile\.am:1:.*'generic/b\.c'.* in a subdirectory" stderr
grep "option 'subdir-objects' is disabled" stderr
# Verbose tips should be given, but not too many times.
for msg in \
  "possible forward-incompatibility" \
  "advi[sc]e.* 'subdir-objects' option throughout" \
  "unconditionally.* object file.* same subdirectory" \
; do
  test $(grep -c "$msg" stderr) -eq 1
done

# Guard against stupid typos.
grep 'subdir-object([^s]|$)' stderr && exit 1

$AUTOMAKE -Wno-unsupported

echo AUTOMAKE_OPTIONS = subdir-objects >> Makefile.am
AUTOMAKE_fails
grep "^Makefile\.am" stderr && exit 1
grep "^sub/Makefile\.am:.*'generic/b\.c'.* in a subdirectory" stderr
grep "option 'subdir-objects' is disabled" stderr

sed 's/^AM_INIT_AUTOMAKE/&([subdir-objects])/' configure.ac > configure.tmp
mv -f configure.tmp configure.ac
$ACLOCAL --force
$AUTOMAKE

rm -f compile
$AUTOMAKE --add-missing 2>stderr || { cat stderr >&2; exit 1; }
cat stderr >&2
# Make sure compile is installed, and that Automake says so.
grep '^configure\.ac:[48]:.*install.*compile' stderr
test -f compile

grep '^generic/a\.\$(OBJEXT):' Makefile.in
grep '^generic/b\.\$(OBJEXT):' sub/Makefile.in
grep '^another/z\.\$(OBJEXT):' Makefile.in
$EGREP '(^|[^/])[abz]\.\$(OBJEXT)' Makefile.in sub/Makefile.in && exit 1

# Opportunistically test for a different bug.
grep '^another/z\.\$(OBJEXT):.*dirstamp' Makefile.in
grep '^generic/b\.\$(OBJEXT):.*dirstamp' sub/Makefile.in

:
