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

# Texinfo input files using @include directives.  Check both in-tree
# and VPATH builds, and both top-level and subdir input.

required='makeinfo tex texi2dvi'
. test-init.sh

echo AC_OUTPUT >> configure.ac

cat > Makefile.am << 'END'
info_TEXINFOS = main.texi sub/more.texi
main_TEXINFOS = one.texi two.texi three.texi
sub_more_TEXINFOS = sub/desc.texi sub/hist.texi
END

cat > main.texi << 'END'
\input texinfo
@setfilename main.info
@settitle main
@ifnottex
@node Top
@top GNU dummy.
@menu
* one::   Chapter one
* two::   Chapter two
* three:: Chapter three
@end menu
@end ifnottex
@include one.texi
@include two.texi
@include three.texi
@bye
END

cat > one.texi << 'END'
@node one
@chapter Chapter one
Foo bar, baz.
END

cat > two.texi << 'END'
@node two
@chapter Chapter two
Blah Blah Blah.
END

cat > three.texi << 'END'
@node three
@chapter Chapter two
GNU's Not Unix.
END

mkdir sub

cat > sub/more.texi << 'END'
\input texinfo
@setfilename more.info
@settitle main
@ifnottex
@node Top
@top GNU more.
@menu
* desc:: Description of this program
* hist:: History of this program
@end menu
@end ifnottex
@include desc.texi
@include hist.texi
@bye
END

cat > sub/desc.texi << 'END'
@node desc
@chapter Description of this program
It does something, really.
END

cat > sub/hist.texi << 'END'
@node hist
@chapter History of this program
It was written somehow.
END

cat > exp << 'END'
./main.info
./sub/more.info
END

check_info_contents ()
{
  srcdir=${1-.}
  $FGREP "Foo bar, baz."                        $srcdir/main.info
  $FGREP "Blah Blah Blah."                      $srcdir/main.info
  $FGREP "GNU's Not Unix."                      $srcdir/main.info
  $FGREP 'It does something, really.'           $srcdir/sub/more.info
  $FGREP 'It was written somehow.'              $srcdir/sub/more.info
}

get_info_names ()
{
  find ${1-.} -type f -name '*.info' | LC_ALL=C sort > got
}

check_expected ()
{
  cat exp
  cat got
  diff exp got
}

$ACLOCAL
$AUTOMAKE --add-missing
$AUTOCONF

./configure

$MAKE info
get_info_names
check_expected

check_info_contents

$MAKE dvi
test -f main.dvi
test -f sub/more.dvi

$MAKE maintainer-clean
test ! -f main.dvi
test ! -f sub/more.dvi
test ! -f main.info
test ! -f sub/more.info

mkdir build
cd build
../configure
$MAKE all dvi

get_info_names ..
sed 's|^\./|../|' ../exp > exp
check_expected

test -f main.dvi
test -f sub/more.dvi

check_info_contents ..

$MAKE distcheck

:
