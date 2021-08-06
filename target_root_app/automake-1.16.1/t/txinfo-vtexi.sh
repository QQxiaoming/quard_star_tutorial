#!/bin/sh
# Copyright (C) 1996-2018 Free Software Foundation, Inc.
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

# Basic checks and some regressions testing on 'version.texi'
# support for texinfo files.

. test-init.sh

cat > Makefile.am << 'END'
info_TEXINFOS = textutils.texi
END

cat > textutils.texi << 'END'
@include version.texi
@setfilename textutils.info
END

# Required when using Texinfo.
: > mdate-sh
: > texinfo.tex

$ACLOCAL
$AUTOMAKE

# Test for bug reported by Jim Meyering:
# When I ran automake-0.29 on textutils,
# I noticed that doc/Makefile.in had
#   textutils.info: textutils.texi
# instead of
#   textutils.info: textutils.texi version.texi
# Today this should be:
#   $(srcdir)/textutils.info: $(srcdir)/version.texi
# or:
#   $(srcdir)/textutils.info: version.texi
grep '^\$(srcdir)/textutils\.info:.*[ /]version\.texi *$' Makefile.in

# Test for bug reported by Lars Hecking:
# When running the first version of configure.ac aware automake,
# @CONFIGURE_AC@ was not properly substituted.
$EGREP 'stamp-vti:.*textutils\.texi( .*)?$' Makefile.in
$EGREP 'stamp-vti:.*\$\(top_srcdir\)/configure( .*)?$' Makefile.in

# Check that the path to mdate-sh is correct.  Over escaping of '$'
# etc. once led to '\$\(srcdir\)/mdate-sh'.
# Filter out '$(srcdir)/mdate-sh'; there should be no occurrences
# of '.../mdate-sh' left then.
sed 's,\$(srcdir)/mdate-sh,,g' Makefile.in | grep '/mdate-sh' && exit 1

:
