#! /bin/sh
# Copyright (C) 1997-2018 Free Software Foundation, Inc.
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

# Test to make sure .info-less @setfilename works.

required='makeinfo tex texi2dvi'
. test-init.sh

cat >> configure.ac << 'END'
AC_OUTPUT
END

cat > Makefile.am << 'END'
info_TEXINFOS = textutils.texi
END

cat > textutils.texi <<EOF
\input texinfo
@c dummy comment
@setfilename textutils
@settitle main
@node Top
Hello walls.
@bye
EOF

$ACLOCAL
$AUTOCONF
AUTOMAKE_fails -a
$EGREP '^(\./)?textutils\.texi:3:.*suffix-less info file' stderr

$AUTOMAKE -a -Wno-obsolete

grep '^INFO_DEPS.*textutils$' Makefile.in

# We should not use single suffix inference rules (with separate
# dependencies), this confuses Solaris make.
grep '^\.texi:$' Makefile.in && exit 1
grep 'textutils: *textutils\.texi' Makefile.in

./configure
$MAKE distcheck

:
