#! /bin/sh
# Copyright (C) 2012-2018 Free Software Foundation, Inc.
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

# The info, html, pdf, ps and dvi targets shouldn't let clutter in the
# build directory.  Related to automake bug#11146.

required='makeinfo tex texi2dvi dvips'
. test-init.sh

cat >> configure.ac <<'END'
AC_CONFIG_FILES([sub/Makefile])
AC_OUTPUT
END

cat > Makefile.am << 'END'
all-local: ps pdf dvi html # For "make distcheck".
info_TEXINFOS = foo.texi doc/bar.texi baz.texi
SUBDIRS = sub
END

mkdir sub doc

cat > sub/Makefile.am << 'END'
all-local: ps pdf dvi html # For "make distcheck".
info_TEXINFOS = baz.texi
END

cat > foo.texi << 'END'
\input texinfo
@setfilename foo.info
@settitle foo
@node Top
Hello walls.
@include version.texi
@bye
END

cat > doc/bar.texi << 'END'
\input texinfo
@setfilename bar.info
@settitle bar
@node Top
Hello walls.
@include version2.texi
@bye
END

cat > baz.texi << 'END'
\input texinfo
@setfilename baz.info
@settitle baz
@defindex au
@defindex sa
@defindex sb
@node Top
Hello walls.
@cindex foo
foo
@pindex bar
bar
@auindex baz
baz
@saindex sa
sa
@sbindex sb
sb
@bye
END

cp baz.texi sub

$ACLOCAL
$AUTOMAKE --add-missing
$AUTOCONF

./configure

# Try one by one, to ensure later targets don't involuntarily
# clean up potential cruft left by earlier ones.
for fmt in info pdf ps dvi html all; do
  $MAKE $fmt
  # For debugging.
  ls -l . doc sub
  # Sanity check.
  case $fmt in
    html)
      test -e foo.html
      test -e doc/bar.html
      test -e baz.html
      test -e sub/baz.html
      ;;
    all)
      for x in info pdf ps dvi; do
        test -f foo.$x
        test -f doc/bar.$x
        test -f baz.$x
        test -f sub/baz.$x
      done
      test -e foo.html
      test -e doc/bar.html
      test -e baz.html
      test -e sub/baz.html
      ;;
    *)
      test -f foo.$fmt
      test -f doc/bar.$fmt
      test -f baz.$fmt
      test -f sub/baz.$fmt
      ;;
  esac
  # Real test.
  ls -d foo* baz* sub/baz* doc/bar* > lst
  basename_rx='(foo|doc/bar|baz|sub/baz)'
  case $fmt in
    pdf) extension_rx="(texi|pdf|t2p)";;
    dvi) extension_rx="(texi|dvi|t2d)";;
     ps) extension_rx="(texi|ps|dvi|t2d)";;
   info) extension_rx="(texi|info)";;
   html) extension_rx="(texi|html)";;
    all) extension_rx="(texi|html|info|pdf|ps|dvi|t2[pd])";;
      *) fatal_ "unreachable code reached";;
  esac
  $EGREP -v "^$basename_rx\.$extension_rx$" lst && exit 1
  # Cleanup for checks on the next format.
  case $fmt in
    info) rm -f *.info doc/*.info sub/*.info;;
       *) $MAKE clean;;
  esac
done

$MAKE distcheck

:
