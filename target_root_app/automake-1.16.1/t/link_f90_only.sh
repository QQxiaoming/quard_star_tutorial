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

# Test to make sure the Fortran 90 linker is used when appropriate.
# (copied from 'link_f_only.sh') Mike Nolta <mrnolta@princeton.edu>

. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_FC
END

cat > Makefile.am << 'END'
bin_PROGRAMS = lavalamp
lavalamp_SOURCES = lamp.f90
END

$ACLOCAL
$AUTOMAKE

# We should only see the Fortran linker in the rules of 'Makefile.in'.

# Look for this macro not at the beginning of any line; that will have
# to be good enough for now.
grep '.\$(FCLINK)' Makefile.in

# We should not see these patterns:
grep '.\$(CXXLINK)' Makefile.in && exit 1
grep '.\$(LINK)'    Makefile.in && exit 1

exit 0
