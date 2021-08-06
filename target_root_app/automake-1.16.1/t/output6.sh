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

# Check for support for colon-separated input files in AC_CONFIG_FILES.

. test-init.sh

cat > configure.ac << END
AC_INIT([$me], [1.0])
AM_INIT_AUTOMAKE
AC_CONFIG_FILES([foo:a.in:b.in:c.in:d.in])
AC_OUTPUT
END

cat >a.in <<'EOF'
a = A
b = A
c = A
d = A
EOF

cat >b.in <<'EOF'
b = B
c = B
d = B
EOF

cat >c.am <<'EOF'
c = C
d = C

.PHONY: test1 test2
test1:
	test $(a)$(b)$(c)$(d) = ABCD
test2:
	test $(a)$(b)$(c)$(d) = GFCD
EOF

echo 'd = D' > d.in


$ACLOCAL
$AUTOCONF
$AUTOMAKE
./configure
$MAKE -f foo test1

$sleep
cat >b.in <<'EOF'
a = G
b = F
c = F
d = F
EOF

using_gmake || $MAKE -f foo
$MAKE -f foo test2

:
