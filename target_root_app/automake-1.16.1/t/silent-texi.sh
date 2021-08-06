#!/bin/sh
# Copyright (C) 2009-2018 Free Software Foundation, Inc.
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

# Check texinfo rules in silent-rules mode.

required='makeinfo tex texi2dvi dvips'
. test-init.sh

echo AC_OUTPUT >> configure.ac

cat > Makefile.am <<'EOF'
info_TEXINFOS = foo.texi sub/zardoz.texi
EOF

cat > foo.texi <<'EOF'
\input texinfo
@setfilename foo.info
@settitle foo manual
@bye
EOF

mkdir sub
cat > sub/zardoz.texi <<'EOF'
\input texinfo
@setfilename zardoz.info
@settitle zardoz manual
@bye
EOF

$ACLOCAL
$AUTOMAKE --add-missing
$AUTOCONF

./configure --disable-silent-rules

# Silent mode output.
run_make -O -E -- V=0 dvi html info ps pdf
grep '^  DVIPS    foo\.ps$'         stdout
grep '^  MAKEINFO foo\.html$'       stdout
# NetBSD make will print './foo.info' instead of 'foo.info'.
$EGREP '^  MAKEINFO (\./)?foo\.info$' stdout
grep '^  TEXI2DVI foo\.dvi$'        stdout
grep '^  TEXI2PDF foo\.pdf$'        stdout
grep '^  DVIPS    sub/zardoz.ps$'   stdout
grep '^  MAKEINFO sub/zardoz.html$' stdout
# NetBSD make will print './sub/zardoz.info' instead of 'zardoz.info'.
$EGREP '^  MAKEINFO (\./)?sub/zardoz.info$' stdout
grep '^  TEXI2DVI sub/zardoz.dvi$'  stdout
grep '^  TEXI2PDF sub/zardoz.pdf$'  stdout
# No make recipe is displayed before being executed.
$EGREP 'texi2(dvi|pdf)|dvips|makeinfo|(rm|mv) ' \
  stdout stderr && exit 1
# No verbose output from TeX nor dvips.
$EGREP '(zardoz|foo)\.log|3\.14|Copyright|This is|[Oo]utput ' \
  stdout stderr && exit 1

# Verbose mode output.
$MAKE clean || exit 1
run_make -M V=1 dvi html info ps pdf
$EGREP '(DVIPS|MAKEINFO|TEXI2(PDF|DVI)) ' output && exit 1
# Verbose output from TeX.
grep '[Oo]utput .*foo\.pdf' output
grep '[Oo]utput .*zardoz\.pdf' output
$FGREP 'foo.log' output
$FGREP 'zardoz.log' output
# Verbose output from dvips.
$FGREP ' dvips' output

:
