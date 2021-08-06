#! /bin/sh
# Copyright (C) 2013-2018 Free Software Foundation, Inc.
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

# Basic tests for '%...%' preprocessing in included Makefile fragments:
#   %reldir%        a.k.a.  %D%
#   %canon_reldir%  a.k.a.  %C%

. test-init.sh

cat >> configure.ac << 'END'
AC_CONFIG_FILES([zot/Makefile])
AC_OUTPUT
END

mkdir foo foo/bar foo/foobar zot

cat > Makefile.am << 'END'
include $(top_srcdir)/foo/local.mk
include $(srcdir)/foo/foobar/local.mk
include local.mk
END

cat > zot/Makefile.am << 'END'
include $(top_srcdir)/zot/local.mk

## Check that '%canon_reldir%' doesn't remain overridden
## by the previous include.
%canon_reldir%_zot_whoami:
	echo "I am %reldir%/Makefile.am" >$@

include $(top_srcdir)/top.mk
include ../reltop.mk
END

cat > local.mk << 'END'
%canon_reldir%_whoami:
	echo "I am %reldir%/local.mk" >$@
END

cat > top.mk << 'END'
%canon_reldir%_top_whoami:
	echo "I am %reldir%/top.mk" >$@
END

cat > reltop.mk << 'END'
%C%_reltop_whoami:
	echo "I am %D%/reltop.mk" >$@
END

cp local.mk foo
cp local.mk foo/bar
cp local.mk foo/foobar
cp local.mk zot

cat >> foo/local.mk << 'END'
include %reldir%/bar/local.mk
## Check that '%canon_reldir%' doesn't remain overridden by the
## previous include.  The duplicated checks are done to ensure that
## Automake substitutes all pre-processing occurrences on a line,
## not just the first one.
test-%reldir%:
	test '%reldir%'       = foo  &&  test '%reldir%' = foo
	test '%D%'            = foo  &&  test '%D%'      = foo
	test '%canon_reldir%' = foo  &&  test '%C%'      = foo
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE
./configure

check ()
{
  test $# -eq 2 || fatal_ "made_into(): bad usage"
  target=$1 contents=$2
  rm -f "$target" \
   && $MAKE "$target" \
   && test x"$(cat "$target")" = x"$contents"
}

check whoami "I am local.mk"
check foo_whoami "I am foo/local.mk"
check foo_bar_whoami "I am foo/bar/local.mk"
check foo_foobar_whoami "I am foo/foobar/local.mk"
$MAKE test-foo

cd zot
check whoami "I am local.mk"
check ___top_whoami "I am ../top.mk"
check ___reltop_whoami "I am ../reltop.mk"
check zot_whoami "I am Makefile.am"

:
