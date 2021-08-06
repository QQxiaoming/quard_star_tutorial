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

# Check that 'dist-hook' works.  See automake bug#10878.

. test-init.sh

echo AC_OUTPUT >> configure.ac

cat > Makefile.am <<'END'
EXTRA_DIST = write execute removed doc

removed:
	echo I will be deleted > $@
DISTCLEANFILES = removed

dist-hook:
	chmod u+w $(distdir)/write $(distdir)/doc
	chmod u+x $(distdir)/execute
	rm -f $(distdir)/removed
	rm -f $(distdir)/doc/HACKING
	rm -f $(distdir)/doc/RELEASE-DATE
	date > $(distdir)/doc/RELEASE-DATE
	echo all is ok > $(distdir)/write

check-local:
	ls -l $(srcdir) $(srcdir)/doc
	test "`cat $(srcdir)/write`" = "all is ok"
	test -f $(srcdir)/doc/README
	test -f $(srcdir)/doc/RELEASE-DATE
	test ! -f $(srcdir)/removed
	test ! -r $(srcdir)/removed
	test ! -f $(srcdir)/doc/HACKING
	test ! -r $(srcdir)/doc/HACKING
	$(srcdir)/execute
	$(srcdir)/execute | grep 'I run successfully'
## Sanity check.
	echo ok > $(srcdir)/../distcheck-run
END

$ACLOCAL
$AUTOMAKE
$AUTOCONF

./configure
mkdir doc
: > doc/README
: > doc/HACKING
echo will be clobbered > write
cat > execute <<'END'
#!/bin/sh
echo I run successfully
END

chmod a-w write
chmod a-x execute

$MAKE distdir
ls -l $distdir $distdir/doc
cd $distdir
test "$(cat write)" = "all is ok"
test ! -e removed
test -f doc/README
test -f doc/RELEASE-DATE
test ! -e doc/HACING
./execute
./execute | grep 'I run successfully'
cd ..

$MAKE distcheck
test -f distcheck-run

:
