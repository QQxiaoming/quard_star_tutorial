#! /bin/sh
# Copyright (C) 2002-2018 Free Software Foundation, Inc.
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

# Test for locations in error messages.

. test-init.sh

cat >> configure.ac << 'END'
AM_CONDITIONAL([COND1], [true])
AM_CONDITIONAL([COND2], [true])
AC_PROG_CC
AM_PROG_AR
AC_PROG_RANLIB
END

cat > Makefile.am << 'END'
bin_PROGRAMS = libfoo.a
if COND2
  lib_LIBRARIES = libfoo.a
endif
if COND1
  bin_PROGRAMS += ctags
endif
VAR = 1 \
      2 \
      3

VAR = 1 \
      2 \
      3
END

: > ar-lib

# Smash the useless difference of lib file locations.
smash_useless_diffs ()
{
  sed -e "s,^$am_amdir/\\([a-z]*\.am\\),\\1," \
      -e "s,^automake-$APIVERSION:,automake:," ${1+"$@"};
}

$ACLOCAL
AUTOMAKE_fails -Wno-error

smash_useless_diffs stderr >observed

# Apparently useless use of sed here required to avoid spuriously
# triggering some maintainer-checks failures.
sed 's/^> //' > expected << 'END'
> Makefile.am:12: warning: VAR multiply defined in condition TRUE ...
> Makefile.am:8: ... 'VAR' previously defined here
> automake: error: libfoo_a_OBJECTS should not be defined
> Makefile.am:3:   while processing library 'libfoo.a'
> automake: error: use 'libfoo_a_LDADD', not 'libfoo_a_LIBADD'
> Makefile.am:3:   while processing library 'libfoo.a'
> library.am: warning: deprecated feature: target 'libfoo.a' overrides 'libfoo.a$(EXEEXT)'
> library.am: change your target to read 'libfoo.a$(EXEEXT)'
> Makefile.am:3:   while processing library 'libfoo.a'
> program.am: target 'libfoo.a$(EXEEXT)' was defined here
> Makefile.am:1:   while processing program 'libfoo.a'
> program.am: warning: redefinition of 'libfoo.a$(EXEEXT)' ...
> Makefile.am:1:   while processing program 'libfoo.a'
> library.am: ... 'libfoo.a' previously defined here
> Makefile.am:3:   while processing library 'libfoo.a'
> tags.am: warning: redefinition of 'ctags' ...
> program.am: ... 'ctags$(EXEEXT)' previously defined here
> Makefile.am:6:   while processing program 'ctags'
END

cat expected
cat observed
diff expected observed || exit 1

AUTOMAKE_fails -Werror
smash_useless_diffs stderr >observed
(echo 'automake: warnings are treated as errors' && cat expected) > t
mv -f t expected

cat expected
cat observed

diff expected observed || exit 1

:
