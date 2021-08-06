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

# Make sure we only create texinfo-related targets once.

. test-init.sh

cat > Makefile.am << 'END'
info_TEXINFOS = maude.texi liver.texi heart.texi
END

echo '@setfilename maude.info' > maude.texi
echo '@setfilename liver.info' > liver.texi
echo '@setfilename heart.info' > heart.texi
: > texinfo.tex

$ACLOCAL
$AUTOMAKE

# These are just examples -- basically for many targets in texinfos.am
# we only want them to appear once.  But grepping them all would be
# overkill.
for t in info dist-info dvi-am install-html uninstall-pdf-am; do
  $EGREP "(^| )$t*.:" Makefile.in # For debugging.
  test $($EGREP -c "(^| )$t(:| *.:)" Makefile.in) -eq 1
done

:
