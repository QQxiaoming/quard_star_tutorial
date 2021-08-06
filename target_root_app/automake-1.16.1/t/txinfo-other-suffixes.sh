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

# Test to make sure '.txi' and '.texinfo' extensions are deprecated,
# but still work.

. test-init.sh

cat > Makefile.am << 'END'
info_TEXINFOS = foo.txi bar.texinfo
END

echo '@setfilename foo.info' > foo.txi
echo '@setfilename bar.info' > bar.texinfo
: > texinfo.tex

$ACLOCAL
AUTOMAKE_fails
grep "^Makefile\.am:.*suffix '.txi'.*Texinfo file.*discouraged" stderr
grep "^Makefile\.am:.*suffix '.texinfo'.*Texinfo file.*discouraged" stderr
grep "^Makefile\.am:.* use '.texi' instead" stderr

$AUTOMAKE -Wno-obsolete

grep '^\.txi\.info: *$' Makefile.in
grep '^\.texinfo\.info: *$' Makefile.in

:
