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

# Test that automake complains properly when the files with unrecognized
# extensions are passed to the TEXINFOS primary.

. test-init.sh

cat > Makefile.am << 'END'
info_TEXINFOS = foobar.foo bazquux.tex zardoz.c
END

echo '@setfilename foobar.info'  > foobar.foo
echo '@setfilename bazquux.info' > bazquux.tex
echo '@setfilename zardoz.info'  > zardoz.c
: > texinfo.tex

$ACLOCAL
AUTOMAKE_fails

for x in 'foobar\.foo' 'bazquux\.tex' 'zardoz\.c'; do
  grep "^Makefile\\.am:.* texinfo file .*$x.* unrecognized extension" stderr
done

:
