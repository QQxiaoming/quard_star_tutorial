#! /bin/sh
# Copyright (C) 2002-2018 Free Software Foundation, Inc.
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

# Make sure that Automake suggest using AM_PATH_LISPDIR to define lispdir.

. test-init.sh


: TRY 1 -- We lack both EMACS and lispdir.

cat > Makefile.am << 'END'
lisp_LISP = foo.el
END

$ACLOCAL
AUTOMAKE_fails
grep AM_PATH_LISPDIR stderr
grep '[Ll]isp source.*EMACS.* undefined' stderr
grep '[Ll]isp source.*lispdir.* undefined' stderr
grep ' add .*AM_PATH_LISPDIR' stderr


: TRY 2 -- Setting lispdir should not be enough.

cat > Makefile.am << 'END'
lispdir = /usr/share/emacs/site-lisp
lisp_LISP = foo.el
END

$ACLOCAL
AUTOMAKE_fails
grep 'lispdir.*undefined' stderr && exit 1
grep '[Ll]isp source.*EMACS.* undefined' stderr
grep 'define .*EMACS.* add .*AM_PATH_LISPDIR' stderr


: TRY 3 -- Setting EMACS should not be enough.

cat > Makefile.am << 'END'
EMACS = emacs
lisp_LISP = foo.el
END

$ACLOCAL
AUTOMAKE_fails
grep 'EMACS.*undefined' stderr && exit 1
grep '[Ll]isp source.*lispdir.* undefined' stderr
grep 'define .*lispdir.* add .*AM_PATH_LISPDIR' stderr


: TRY 4 -- Setting both EMACS and lispdir is OK.

cat > Makefile.am << 'END'
lispdir = /usr/share/emacs/site-lisp
EMACS = emacs
lisp_LISP = foo.el
END

$ACLOCAL
$AUTOMAKE -a

:
