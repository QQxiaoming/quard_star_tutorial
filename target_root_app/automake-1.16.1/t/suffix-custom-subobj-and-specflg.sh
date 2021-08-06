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

# Tests that Automake understands suffix rules with renamed objects
# and subdir objects.
# Reported by Florian Briegel.

required=cc
. test-init.sh

cat >>configure.ac <<EOF
AC_PROG_CC
AC_OUTPUT
EOF

cat >Makefile.am << 'END'
AUTOMAKE_OPTIONS = subdir-objects
SUFFIXES = .baz .c
.baz.c:
	case $@ in sub/*) $(MKDIR_P) sub;; *) :;; esac
## Account for VPATH issues on weaker make implementations (e.g. IRIX 6.5).
	cp `test -f '$<' || echo $(srcdir)/`$< $@

DISTCLEANFILES = sub/bar.c

bin_PROGRAMS = foo
foo_SOURCES = foo.c sub/bar.baz
foo_CFLAGS =
END

mkdir sub
cat > sub/bar.baz <<'END'
extern int foo ();
int main () { return foo (); }
END
cat > foo.c <<'END'
int foo () { return 0; }
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure

$MAKE
$MAKE distcheck

:
