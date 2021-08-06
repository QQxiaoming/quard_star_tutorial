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

# Expose bug in conditional definition of TEST_EXTENSIONS.

. test-init.sh

cat >> configure.ac << 'END'
AM_CONDITIONAL([COND], [:])
AM_CONDITIONAL([COND2], [:])
END

$ACLOCAL

cat > 1.am << 'END'
TESTS =
if COND
## lineno 4
TEST_EXTENSIONS = .foo
endif
END

cat > 2.am << 'END'
TESTS =
## lineno 3
TEST_EXTENSIONS = .foo
if COND
# Do nothing.
else
TEST_EXTENSIONS += .bar
endif
END

cat > 3.am << 'END'
TESTS =
if COND
if !COND2
TESTS = x
else
## lineno 7
TEST_EXTENSIONS = .foo
endif
endif
END

: > test-driver

for i in 1 2 3; do
  AUTOMAKE_fails $i
  lineno=$(sed -n 's/^## lineno //p' $i.am) \
    && test 0 -lt "$lineno" \
    || exit 99
  grep "^$i\\.am:$lineno:.*TEST_EXTENSIONS.*conditional content" stderr
done

:
