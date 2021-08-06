#! /bin/sh
# Copyright (C) 2006-2018 Free Software Foundation, Inc.
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

# Make sure file extensions are matched correctly in the code
# parsing texi files for @setfilename declarations.
# Report from Eric Dorland.

. test-init.sh

echo info_TEXINFOS = bar.texi >Makefile.am
cat >bar.texi <<EOF
@setfilename bar-1.9.info
EOF

$ACLOCAL
$AUTOMAKE --add-missing

$EGREP '[ /]bar-1\.9\.info( |$)' Makefile.in

:
