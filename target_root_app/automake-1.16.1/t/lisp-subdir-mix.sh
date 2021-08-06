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

# Check compiling elisp files in different subdirectories, where a
# file in a subdirectory might require a file in another one.
# This doesn't work out of the box, but can be made to work with a
# judicious use of $(AM_ELCFLAGS).

required=emacs
. test-init.sh

cat >> configure.ac << 'END'
AM_PATH_LISPDIR
AC_OUTPUT
END

cat > Makefile.am << 'END'
dist_lisp_LISP = \
  am-here.el \
  sub1/am-one.el \
  sub2/am-two.el \
  sub3/subsub/am-three.el

AM_ELCFLAGS = \
  -L $(srcdir)/sub1 \
  -L $(srcdir)/sub2 \
  -L $(srcdir)/sub3/subsub

elc-test:
	test -f sub1/am-one.elc
	test -f sub2/am-two.elc
	test -f sub3/subsub/am-three.elc
.PHONY: elc-test
check-local: elc-test
END

mkdir sub1 sub2 sub3 sub3/subsub

cat > am-here.el << 'END'
(provide 'am-here)
(require 'am-one)
(require 'am-two)
(require 'am-three)
END

cat > sub1/am-one.el << 'END'
(require 'am-here)
(provide 'am-one)
(require 'am-two)
(require 'am-three)
END

cat > sub2/am-two.el << 'END'
(require 'am-here)
(require 'am-one)
(provide 'am-two)
(require 'am-three)
END

cat > sub3/subsub/am-three.el << 'END'
(require 'am-here)
(require 'am-one)
(require 'am-two)
(provide 'am-three)
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE --add-missing

./configure

$MAKE
$MAKE elc-test
$MAKE clean
for x in am-here sub1/am-one sub2/am-two sub3/subsub/am-three; do
  test -f $x.el
  test ! -e $x.elc
done

$MAKE distcheck

:
