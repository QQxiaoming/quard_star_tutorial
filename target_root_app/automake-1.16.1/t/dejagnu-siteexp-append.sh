#! /bin/sh
# Copyright (C) 2011-2018 Free Software Foundation, Inc.
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

# Check that the files in $(EXTRA_DEJAGNU_SITE_CONFIG) get appended to
# site.exp in the same order in which they're listed in that variable.

. test-init.sh

cat >> configure.ac << 'END'
AC_OUTPUT
END

cat > Makefile.am << 'END'
AUTOMAKE_OPTIONS = dejagnu
DEJATOOL = tool
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE --add-missing

./configure

$MAKE site.exp
sed '/^##.*##$/d' site.exp > expected

cat > foo.exp << 'END'
foo1
foo2
foo3
END

cat foo.exp - >> expected << 'END'
BAR 1
# foo
BAR 2
END

cat >> Makefile.am << 'END'
EXTRA_DEJAGNU_SITE_CONFIG = foo.exp bar.exp
bar.exp:
	@(echo 'BAR 1' && echo '# foo' && echo 'BAR 2') > $@
END

$AUTOMAKE Makefile
./config.status Makefile

rm -f site.exp
$MAKE site.exp
sed '/^##.*##$/d' site.exp > obtained

cat expected
cat site.exp

diff expected obtained

:
