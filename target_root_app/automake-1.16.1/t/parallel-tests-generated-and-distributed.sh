#! /bin/sh
# Copyright (C) 2009-2018 Free Software Foundation, Inc.
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

# Check parallel-tests features:
# - generated distributed tests.
# - listing $(srcdir)/ or $(top_srcdir)/ in TESTS doesn't work ATM,
#   and is thus diagnosed.

# TODO: this test should also ensure that the 'make' implementation
#       properly adheres to rules in all cases.  See the Autoconf
#       manual for the ugliness in this area, when VPATH comes into
#       play.  :-/

. test-init.sh

cat >> configure.ac << 'END'
AC_OUTPUT
END

cat > Makefile.am << 'END'
TESTS = foo.test
## the next line will cause automake to error out:
TESTS += $(srcdir)/bar.test $(top_srcdir)/baz.test
.in.test:
## Account for VPATH issues on weaker make implementations (e.g. IRIX 6.5)
	cp `test -f '$<' || echo $(srcdir)/`$< $@
	chmod +x $@
check_SCRIPTS = $(TESTS)
EXTRA_DIST = foo.in foo.test
DISTCLEANFILES = foo.test
END

cat > foo.in <<'END'
#! /bin/sh
echo "this is $0"
exit 0
END

$ACLOCAL
$AUTOCONF
AUTOMAKE_fails -a
grep '$(srcdir).*TESTS.*bar\.test' stderr
grep '$(top_srcdir).*TESTS.*baz\.test' stderr

sed '/^TESTS +=.*srcdir/d' < Makefile.am > t
mv -f t Makefile.am
$AUTOMAKE -a

./configure
$MAKE check
$MAKE distcheck
$MAKE distclean

mkdir build
cd build
../configure
$MAKE check
test ! -e ../foo.log
$MAKE distcheck

:
