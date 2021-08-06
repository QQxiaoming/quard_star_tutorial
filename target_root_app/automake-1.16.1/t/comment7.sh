#! /bin/sh
# Copyright (C) 2002-2018 Free Software Foundation, Inc.
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

# Make sure comment for conditional variables are output near the
# corresponding conditional definitions.

. test-init.sh

cat >> configure.ac <<'EOF'
AM_CONDITIONAL([COND], [true])
EOF

cat > Makefile.am << 'EOF'
if COND
# Comment for VAR in COND_TRUE.
VAR = foo
else
# Comment for VAR in COND_FALSE.
VAR = bar
endif
EOF

$ACLOCAL
$AUTOMAKE

$FGREP '@COND' Makefile.in # For debugging, mostly.

# The VAR definition appears once for each condition.
test $(grep -c '@COND_TRUE@VAR' Makefile.in) = 1
test $(grep -c '@COND_FALSE@VAR' Makefile.in) = 1

# Make sure the right definition follows each comment.
sed -n '/^#.*VAR.*COND_TRUE/ {
          n
          p
        }' Makefile.in |
  grep '@COND_TRUE@VAR = foo'
sed -n '/^#.*VAR.*COND_FALSE/ {
          n
          p
        }' Makefile.in |
  grep '@COND_FALSE@VAR = bar'

:
