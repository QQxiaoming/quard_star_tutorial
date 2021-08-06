#! /bin/sh
# Copyright (C) 2009-2018 Free Software Foundation, Inc.
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

# Cover corner cases of derive_suffix wrt. file extensions:
# - extension declared with SUFFIXES but with no suffix rule for it
# - extension with a suffix rule but none that leads us to $(OBJEXT)
# In both cases, we don't try to create a rule; but we shouldn't
# fail either, because magic might be happening behind our back.

. test-init.sh

cat >> configure.ac << 'END'
END

cat > Makefile.am << 'END'
bin_PROGRAMS = foo bar
foo_SOURCES = foo.goo
SUFFIXES = .goo
bar_SOURCES = bar.woo
.woo.zoo: ; making a zoo $@ from a woo $<
END

$ACLOCAL
$AUTOMAKE

exit 0
