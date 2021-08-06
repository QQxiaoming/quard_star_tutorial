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

# Make sure the current version can be required.

. test-init.sh

amver=$($AUTOMAKE --version | sed -e 's/.* //;1q')

# Does the extracted version number seems legit?
case $amver in
  *[0-9].[0-9]*) ;;
  *) fatal_ "couldn't extract version number from automake" ;;
esac

cat > Makefile.am << END
AUTOMAKE_OPTIONS = $amver
END

$ACLOCAL
$AUTOMAKE

:
