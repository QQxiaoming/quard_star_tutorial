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

# Make sure dvi target recurses into subdir.
# Reported by Pavel Roskin.

. test-init.sh

cat > Makefile.am << 'END'
SUBDIRS = sub
END

mkdir sub
cat > sub/Makefile.am << 'END'
info_TEXINFOS = maude.texi
END

echo '@setfilename maude.info' > sub/maude.texi
: > sub/texinfo.tex

$ACLOCAL
$AUTOMAKE

grep dvi-recursive Makefile.in
grep '[^-]info-recursive' Makefile.in
grep '[^n]install-info-recursive' Makefile.in
