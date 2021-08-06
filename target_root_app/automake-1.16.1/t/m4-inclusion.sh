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

# Test to see if 'm4_sinclude' and 'm4_include' works.

. test-init.sh

cat >> configure.ac <<'EOF'
sinclude([doesntexist.m4])
EOF

mkdir sub

cat > sub/p.m4 << 'END'
AC_SUBST([MAGICALPIG])
END

cat > sub/h.m4 << 'END'
AC_SUBST([MAGICALHOG])
END

cat > sub/g.m4 << 'END'
AC_SUBST([GREPME])
END

: > Makefile.am

echo 'sinclude([sub/g.m4])' >> acinclude.m4
echo 'm4_sinclude([sub/doesntexist.m4])' >> acinclude.m4

$ACLOCAL

echo 'm4_sinclude([sub/p.m4])' >> aclocal.m4
echo 'm4_include([sub/h.m4])' >> aclocal.m4

$AUTOMAKE

grep doesntexist Makefile.in && exit 1
grep MAGICALPIG Makefile.in
grep MAGICALHOG Makefile.in
grep GREPME Makefile.in

:
