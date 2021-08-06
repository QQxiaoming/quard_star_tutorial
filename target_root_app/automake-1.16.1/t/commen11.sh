#! /bin/sh
# Copyright (C) 2005-2018 Free Software Foundation, Inc.
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

# Make sure backslash-newline-hash combinations are diagnosed as
# comments following a trailing backslash, even when the combination
# follows a variable assignment that is preceded by a comment.

. test-init.sh

cat > Makefile.am << 'END'
# initial comment
variable = value-before-comment \
#

# comment
SUBDIRS = foo \
# bar

END

mkdir foo

$ACLOCAL
AUTOMAKE_fails
grep '^Makefile\.am:3:.*backslash' stderr
grep '^Makefile\.am:7:.*backslash' stderr

:
