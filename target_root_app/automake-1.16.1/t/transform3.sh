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

# Make sure that --program-transform does not transform too much
# stuff (in particular, pgklibdir, pkgdatadir and pkglibexecdir).

required=cc
. test-init.sh

cat > configure.ac <<'END'
AC_INIT([foo], [1.0])
AM_INIT_AUTOMAKE
AC_CONFIG_FILES([Makefile])
AC_PROG_CC
AM_PROG_AR
AC_PROG_RANLIB
AC_OUTPUT
END

cat > Makefile.am <<'END'
bin_SCRIPTS = foo
pkgdata_DATA = bar.txt
pkglib_LIBRARIES = libzap.a
pkglibexec_SCRIPTS = mu

test-install: install
	find $(prefix) ;: For debugging.
	test   -f $(bindir)/gnu-foo
	test   -x $(bindir)/gnu-foo
	test   -f $(datadir)/foo/bar.txt
	test ! -d $(datadir)/gnu-foo
	test   -f $(libdir)/foo/libzap.a
	test ! -d $(libdir)/gnu-foo
	test   -f $(libexecdir)/foo/gnu-mu
	test   -x $(libexecdir)/foo/gnu-mu
	test ! -d $(libexecdir)/gnu-foo

test-installdirs: installdirs
	find $(prefix) ;: For debugging.
	test   -d $(datadir)/foo
	test ! -d $(datadir)/gnu-foo
	test   -d $(libdir)/foo
	test ! -d $(libdir)/gnu-foo
	test   -d $(libexecdir)/foo
	test ! -d $(libexecdir)/gnu-foo
END

cat > libzap.c <<'END'
int zap (void)
{
  return 0;
}
END

echo 'To be or not to be ...' > bar.txt

cat > foo <<'END'
#!/bin/sh
exit 0
END
cp foo mu
chmod a+x foo mu

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure --program-prefix=gnu- --prefix "$(pwd)/inst"

$MAKE test-install

$MAKE uninstall
test $(find inst -type f -print | wc -l) -eq 0

# Opportunistically test for installdirs.
rm -rf inst
$MAKE test-installdirs

:
