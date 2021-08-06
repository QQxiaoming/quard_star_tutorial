#! /bin/sh
# Copyright (C) 1998-2018 Free Software Foundation, Inc.
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

# Test for an odd conditional bug.  Report from Matt Leach.

. test-init.sh

echo 'AM_CONDITIONAL([FOO], [true])' >> configure.ac

cat > Makefile.am << 'END'
if FOO

helpdir = $(prefix)/Help
# The continuation line below must start with a Tab to see the bug.
help_DATA = a b c d e \
	f g h

else

helpdir = $(prefix)/help
help_DATA = foo

endif

a b c d e f g h:
	touch $@

.PHONY: test
test:
	is $(help_DATA) == a b c d e f g h
END


# Older versions of this test checked that automake could process the above
# Makefile.am even with no AC_OUTPUT in configure.  So continue to do this
# check, for completeness.
$ACLOCAL
$AUTOMAKE

rm -rf autom4te*.cache

echo AC_OUTPUT >> configure.ac
touch aclocal.m4 # Avoid unnecessary firing the remake rules.
$AUTOCONF
$AUTOMAKE Makefile

./configure --prefix="$(pwd)/_inst"

$MAKE test

$MAKE install
for x in a b c d e f g h; do
  test -f _inst/Help/$x
done

:
