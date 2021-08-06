#!/bin/sh
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

# Test to make sure that -Wportability turns on portability-recursive,
# likewise for -Wno-...

. test-init.sh

#
# First, try a setup where we have a 'portability-recursive' warning,
# but no "simple" 'portability' warning.
#

cat >Makefile.am <<'EOF'
x = 1
bla = $(foo$(x))
EOF

$ACLOCAL

# Enabling 'portability' warnings should enable 'portability-recursive'
# warnings.
AUTOMAKE_fails -Wnone -Wportability
grep 'recursive variable expansion' stderr
# 'portability-recursive' warnings can be enabled by themselves.
AUTOMAKE_fails -Wnone -Wportability-recursive
grep 'recursive variable expansion' stderr

# Various ways to disable 'portability-recursive'.
$AUTOMAKE -Wno-all
$AUTOMAKE -Wno-portability
$AUTOMAKE -Wall -Wno-portability-recursive

# '-Wno-portability-recursive' after '-Wportability' correctly disables
# 'portability-recursive' warnings.
$AUTOMAKE -Wportability -Wno-portability-recursive

# '-Wno-portability' disables 'portability-recursive' warnings; but
# a later '-Wportability-recursive' re-enables them.  This time, we
# use AUTOMAKE_OPTIONS to specify the warning levels.
echo 'AUTOMAKE_OPTIONS = -Wno-portability' >> Makefile.am
$AUTOMAKE
echo 'AUTOMAKE_OPTIONS += -Wportability-recursive' >> Makefile.am
AUTOMAKE_fails
grep 'recursive variable expansion' stderr

#
# Now try a setup where we have both a 'portability' warning and
# a 'portability-recursive' one.
#

cat >Makefile.am <<'EOF'
x = 1
bla = $(foo$(x))
oops = $(var-with-dash)
EOF

# Can disable both 'portability' and 'portability-recursive' warnings.
$AUTOMAKE -Wno-portability

# Disabling 'portability-recursive' warnings should not disable
# 'portability' warnings.
AUTOMAKE_fails -Wportability -Wno-portability-recursive
grep 'var-with-dash' stderr
grep 'recursive variable expansion' stderr && exit 1

# Enabling 'portability-recursive' warnings should not enable
# all the 'portability' warning.
AUTOMAKE_fails -Wno-portability -Wportability-recursive
grep 'var-with-dash' stderr && exit 1
grep 'recursive variable expansion' stderr

:
