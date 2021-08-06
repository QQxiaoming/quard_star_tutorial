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

# Make sure we warn about possible variable typos when we should.

. test-init.sh

cat >>configure.ac <<'END'
AC_PROG_RANLIB
AC_OUTPUT
END

cat >Makefile.am <<'END'
foo_SOURCES = unused
nodist_foo_SOURCES = unused
EXTRA_foo_SOURCES = unused
foo_LDADD = unused
foo_LDFLAGS = unused
foo_DEPENDENCIES = unused
EXTRA_foo_DEPENDENCIES = unused

libfoo_a_SOURCES = unused
nodist_libfoo_a_SOURCES = unused
EXTRA_libfoo_a_SOURCES = unused
libfoo_a_LIBADD = unused
libfoo_a_DEPENDENCIES = unused
EXTRA_libfoo_a_DEPENDENCIES = unused
END

$ACLOCAL
AUTOMAKE_fails -Wno-extra-portability
# The expected diagnostic is:
# automake: warnings are treated as errors
# Makefile.am:2: warning: variable 'nodist_foo_SOURCES' is defined but no program or
# Makefile.am:2: library has 'foo' as canonical name (possible typo)
# Makefile.am:1: warning: variable 'foo_SOURCES' is defined but no program or
# Makefile.am:1: library has 'foo' as canonical name (possible typo)
# Makefile.am:9: warning: variable 'libfoo_a_SOURCES' is defined but no program or
# Makefile.am:9: library has 'libfoo_a' as canonical name (possible typo)
# Makefile.am:10: warning: variable 'nodist_libfoo_a_SOURCES' is defined but no program or
# Makefile.am:10: library has 'libfoo_a' as canonical name (possible typo)
# Makefile.am:11: warning: variable 'EXTRA_libfoo_a_SOURCES' is defined but no program or
# Makefile.am:11: library has 'libfoo_a' as canonical name (possible typo)
# Makefile.am:3: warning: variable 'EXTRA_foo_SOURCES' is defined but no program or
# Makefile.am:3: library has 'foo' as canonical name (possible typo)
# Makefile.am:12: warning: variable 'libfoo_a_LIBADD' is defined but no program or
# Makefile.am:12: library has 'libfoo_a' as canonical name (possible typo)
# Makefile.am:4: warning: variable 'foo_LDADD' is defined but no program or
# Makefile.am:4: library has 'foo' as canonical name (possible typo)
# Makefile.am:5: warning: variable 'foo_LDFLAGS' is defined but no program or
# Makefile.am:5: library has 'foo' as canonical name (possible typo)
# Makefile.am:14: warning: variable 'EXTRA_libfoo_a_DEPENDENCIES' is defined but no program or
# Makefile.am:14: library has 'libfoo_a' as canonical name (possible typo)
# Makefile.am:7: warning: variable 'EXTRA_foo_DEPENDENCIES' is defined but no program or
# Makefile.am:7: library has 'foo' as canonical name (possible typo)
# Makefile.am:6: warning: variable 'foo_DEPENDENCIES' is defined but no program or
# Makefile.am:6: library has 'foo' as canonical name (possible typo)
# Makefile.am:13: warning: variable 'libfoo_a_DEPENDENCIES' is defined but no program or
# Makefile.am:13: library has 'libfoo_a' as canonical name (possible typo)

grep 'as canonical' stderr | grep -v ' .foo. ' | grep -v ' .libfoo_a. ' \
  && exit 1
test $(grep -c 'variable.*is defined but' stderr) -eq 13

# If we add a global -Wnone, all warnings should disappear.
$AUTOMAKE -Wnone

# Likewise, if matching programs or libraries are defined.
cat >>Makefile.am <<'END'
bin_PROGRAMS = foo
lib_LIBRARIES = libfoo.a
END

$AUTOMAKE -Wno-extra-portability

:
