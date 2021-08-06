#! /bin/sh
# Copyright (C) 2001-2018 Free Software Foundation, Inc.
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

# Test for PR automake/220.
# Test for problems when conditionals are not actually defined.
# Check that the problems is diagnosed by configure.
# This isn't perfect (ideally we'd like an error from autoconf),
# but it is the best we can do.  It certainly makes it easier
# to debug the problem.
# Note that this should be also in the documentation.

. test-init.sh

cat > Makefile.am << 'EOF'
if NEVER_TRUE
NEVER_DEFINED = foo.txt
endif
data_DATA = $(NEVER_DEFINED)
EOF

cat >> configure.ac << 'EOF'
AC_ARG_ENABLE([foo],
[  --enable-foo          Enable foo],
[ if test "foo" = "bar" ; then
    AM_CONDITIONAL([NEVER_TRUE], [true])
  else
    AM_CONDITIONAL([NEVER_TRUE], [false])
  fi
])
AC_OUTPUT
EOF

mkdir build

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

cd build
# configure should fail since we've done something invalid.
../configure 2>stderr && { cat stderr >&2; exit 1; }
cat stderr >&2
grep 'conditional.*NEVER_TRUE' stderr

:
