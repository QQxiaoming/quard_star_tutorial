#! /bin/sh
# Copyright (C) 2012-2018 Free Software Foundation, Inc.
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

# Related to automake bug#12495: Automake shouldn't generate useless
# remake rules for AC_CONFIG_HEADERS arguments after the first one,
# not even when subdirs are involved.

. test-init.sh

cat >> configure.ac << 'END'
AC_CONFIG_HEADERS([a.h b.h sub/c.h])
AC_CONFIG_FILES([sub/Makefile])
AC_OUTPUT
END

mkdir sub
echo SUBDIRS = sub > Makefile.am
: > sub/Makefile.am

$ACLOCAL
$AUTOCONF
$AUTOHEADER
# Even if an AC_CONFIG_HEADERS invocation is passed several files in
# the first argument, only the first one is considered by autoheader
# for automatic generation.  Otherwise, the present test case would
test -f a.h.in && test ! -f c.h.in && test ! -f sub/c.h.in \
  || fatal_ "unexpected autoheader behavior with multiple" \
            "AC_CONFIG_HEADERS arguments"
# Automake should require the missing headers though.
AUTOMAKE_fails -Wno-error -Wnone
grep "^configure\.ac:4:.* required file 'b.h.in' not found" stderr
grep "^configure\.ac:4:.* required file 'sub/c.h.in' not found" stderr
: > b.h.in
: > sub/c.h.in
$AUTOMAKE

./configure

# Automake should regenerate this.
grep '^$(srcdir)/a\.h\.in:' Makefile.in
# But not these.
grep '[bc]\.h\.in.*:' Makefile.in sub/Makefile.in && exit 1

test -f a.h && test -f b.h && test -f sub/c.h \
  || fatal_ "unexpected ./configure behavior with multiple" \
            "AC_CONFIG_HEADERS arguments"

rm -f a.h.in a.h
$MAKE
test -f a.h.in
test -f a.h

# We might need to grep the output of GNU make for error messages.
LANG=C LANGUAGE=C LC_ALL=C
export LANG LANGUAGE LC_ALL

ocwd=$(pwd)
for x in b c; do
  test $x = b || cd sub
  rm -f $x.h.in
  run_make -E -e FAIL $x.h.in
  test ! -f $x.h.in
  if using_gmake; then
    grep "No rule to make target [\`\"']$x\.h\.in[\`\"']" stderr
  fi
  : > $x.h.in
  cd "$ocwd" || fatal_ "cannot chdir back"
done

:
