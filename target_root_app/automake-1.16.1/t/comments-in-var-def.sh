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

# Make sure Automake ignores in-line comments when using variables,
# but preserve them in the output.

. test-init.sh

cat >> configure.ac <<'END'
AC_OUTPUT
END

# Use a slash in the comment, because automake takes the dirname
# of TEXINFO_TEX to compute $(am__TEXINFO_TEX_DIR).
cat > Makefile.am << 'END'
TEXINFO_TEX = tex/texinfo.tex    # some comment w/ a slash
info_TEXINFOS = main.texi
.PHONY: test
test:
	test tex/texinfo.tex = $(TEXINFO_TEX)
	test -d '$(am__TEXINFO_TEX_DIR)'
	case '$(am__TEXINFO_TEX_DIR)' in tex|./tex) :;; *) exit 1;; esac
END

cat > main.texi << 'END'
\input texinfo
@setfilename main.info
END

mkdir tex
: > tex/texinfo.tex

$ACLOCAL
$AUTOMAKE

grep TEX Makefile.in # For debugging.
grep '^TEXINFO_TEX *= *tex/texinfo\.tex  *# some comment w/ a slash *$' Makefile.in

$AUTOCONF
./configure
$MAKE test

:
