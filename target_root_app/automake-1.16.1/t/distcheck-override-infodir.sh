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

# Check that we can override ${infodir} while having distcheck still
# working.  Related to automake bug#9579.

required='makeinfo tex texi2dvi install-info'
. test-init.sh

cat >> configure.ac << 'END'
AC_OUTPUT
END

cat > Makefile.am << 'END'
infodir = ${prefix}/blah/blah/foobar
info_TEXINFOS = main.texi
## Sanity check.
installcheck-local:
	if test x$${infodir+set} != xset; then \
	  ls -l "$(DESTDIR)$(prefix)/blah/blah/foobar/" || exit 1; \
	  test -f "$(DESTDIR)$(prefix)/blah/blah/foobar/dir" || exit 1; \
	else \
	  ls -l "$(DESTDIR)$$infodir/" || exit 1; \
	  test -f "$(DESTDIR)$$infodir/dir" || exit 1; \
	fi
END

# Protect with leading " # " to avoid spurious maintainer-check failures.
sed 's/^ *# *//' > main.texi << 'END'
 # \input texinfo
 # @setfilename main.info
 # @settitle main
 #
 # @c Explicit calls to @dircategory and @direntry required to ensure that
 # @c a 'dir' file will be created also by older versions of 'install-info'
 # @c (e.g., the one coming with Texinfo 4.8).
 #
 # @dircategory Software development
 # @direntry
 # * Automake: (automake).  Making GNU standards-compliant Makefiles
 # @end direntry
 #
 # @dircategory Individual utilities
 # @direntry
 # * aclocal-invocation: (automake)aclocal Invocation.   Generating aclocal.m4
 # * automake-invocation: (automake)automake Invocation. Generating Makefile.in
 # @end direntry
 #
 # @node Top
 # Hello walls.
 # @bye
END

$ACLOCAL
$AUTOMAKE -a
$AUTOCONF

./configure
$MAKE

$MAKE distcheck
run_make infodir="$(pwd)/_info" distcheck
test -f _info/dir || exit 99 # Sanity check.

:
