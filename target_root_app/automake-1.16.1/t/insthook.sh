#! /bin/sh
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

# Make sure the install-exec-hook example we give in the manual works.

. test-init.sh

cat >>configure.ac <<'EOF'
AC_PROG_LN_S
AC_OUTPUT
EOF

cat >Makefile.am <<'END'
dist_bin_SCRIPTS = foo

install-exec-hook:
	cd "$(DESTDIR)$(bindir)" \
	  && mv -f foo foo-$(VERSION) \
	  && $(LN_S) foo-$(VERSION) foo

installcheck-local:
	test -f "$(bindir)/foo"
	test -f "$(bindir)/foo-$(VERSION)"
	: > $(top_srcdir)/../ok

uninstall-hook:
	rm -f $(DESTDIR)$(bindir)/foo-$(VERSION)
END

echo a > foo

$ACLOCAL
$AUTOCONF
$AUTOMAKE

./configure
$MAKE distcheck
# Sanity check to make sure installcheck-local was run.
test -f ok

# Make sure that installing a second version doesn't erase the first
# one.  (This is error prone since 'foo' symlinks to 'foo-1.0' and the
# second version will overwrite 'foo'.  Hopefully 'install' and 'install-sh'
# are smart enough to erase the 'foo' symlink before installing the new
# version.)
./configure "--bindir=$(pwd)/bin"
$MAKE install
echo b > foo
run_make VERSION=2.0 install
test $(cat bin/foo-1.0) = a
test $(cat bin/foo-2.0) = b
test $(cat bin/foo)     = b

# install-hook is an error.
cat >>Makefile.am <<EOF
install-hook:
	echo test
EOF

AUTOMAKE_fails
grep install-data-hook stderr
grep install-exec-hook stderr

:
