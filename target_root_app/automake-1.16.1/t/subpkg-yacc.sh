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

# Check that ylwrap is installed properly, and $(YLWRAP) us defined
# properly, when a subpackage is involved.

required='cc yacc'
. test-init.sh

cat >>configure.ac <<'END'
AC_PROG_CC
AC_CONFIG_SUBDIRS([lib])
AC_OUTPUT
END

cat >Makefile.am <<'EOF'
SUBDIRS = lib
bin_PROGRAMS = MU
MU_LDADD = lib/liblib.a
# It's ok to override distdir.
distdir = subpack-1
# Remove a file created by rules in subdir lib.
CLEANFILES = lib-dist-hook-has-run
EOF

cat >MU.c <<'EOF'
int lib (void);
int main (void)
{
  return lib ();
}
EOF

mkdir lib
mkdir lib/src

cat >lib/configure.ac <<'EOF'
AC_INIT([lib], [2.3])
AM_INIT_AUTOMAKE([subdir-objects])
AC_PROG_RANLIB
AC_PROG_YACC
dnl This comes after YACC and RANLIB checks, deliberately.
AC_PROG_CC
AM_PROG_AR
AC_CONFIG_HEADERS([config.h:config.hin])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
EOF

cat >lib/Makefile.am <<'EOF'
noinst_LIBRARIES = liblib.a
liblib_a_SOURCES = src/x.c foo.y
EXTRA_liblib_a_SOURCES = bar.y

dist-hook:
	test -d $(top_distdir)
	test -d $(distdir)
	find $(top_distdir) $(distdir) ;: For debugging.
	test -f $(top_distdir)/MU.c
	test ! -f $(distdir)/MU.c
	for suf in y c; do \
	  for name in foo bar; do \
	    test -f $(distdir)/$$name.$$suf || exit 1; \
	    test ! -f $(top_distdir)/$$name.$$suf || exit 1; \
	  done; \
	done
	test -f $(distdir)/foo.y
	test ! -f $(top_distdir)/foo.y
	test -f $(distdir)/src/x.c
	test ! -f $(top_distdir)/src/x.c
	test -f $(YLWRAP)
	: > $(top_builddir)/../lib-dist-hook-has-run
EOF

cat > lib/foo.y << 'END'
%{
int yylex (void) { return 0; }
void yyerror (char *s) {}
%}
%%
foobar : 'f' 'o' 'o' 'b' 'a' 'r' {};
END

cp lib/foo.y lib/bar.y

cat >lib/src/x.c <<'EOF'
#include <config.h>
int lib (void)
{
  return 0;
}
EOF

$ACLOCAL
$AUTOCONF
$AUTOMAKE -Wno-override

cd lib
$ACLOCAL
$AUTOCONF
$AUTOHEADER
$AUTOMAKE -Wno-override --add-missing
cd ..

./configure

$MAKE dist
test -f lib-dist-hook-has-run
test -f subpack-1.tar.gz
test ! -e subpack-1 # Make sure "dist" cleans up after itself.

rm -f lib-dist-hook-has-run subpack-1.tar.gz

yl_distcheck

:
