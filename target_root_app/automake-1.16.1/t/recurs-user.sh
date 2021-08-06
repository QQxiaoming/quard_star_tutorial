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

# Basic checks on user-defined recursive rules.
# Check that user recursion respect $(SUBDIRS) order,
# and proceeds in a depth-first fashion.

. test-init.sh

cat >> configure.ac <<'END'
AM_EXTRA_RECURSIVE_TARGETS([foo])
AC_CONFIG_FILES([
    sub1/Makefile
    sub2/Makefile
    sub3/Makefile
    sub3/deeper/Makefile
])
AC_OUTPUT
END

mkdir sub1 sub2 sub3 sub3/deeper

cat > Makefile.am <<'END'
SUBDIRS = sub1 . sub3 sub2
foo-local:
	echo '.' >> $(top_builddir)/got
.PHONY: test
test: foo
	cat $(srcdir)/exp
	cat ./got
	diff $(srcdir)/exp ./got
check-local: test
MOSTLYCLEANFILES = got
EXTRA_DIST = exp
END

for i in 1 2 3; do
  cat > sub$i/Makefile.am <<END
foo-local:
	echo 'sub$i' >> \$(top_builddir)/got
END
done

echo SUBDIRS = deeper >> sub3/Makefile.am

cat >> sub3/deeper/Makefile.am <<'END'
foo-local:
	echo sub3/deeper >> $(top_builddir)/got
END

cat >> exp <<'END'
sub1
.
sub3/deeper
sub3
sub2
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE

./configure

$MAKE test
$MAKE distcheck

:
