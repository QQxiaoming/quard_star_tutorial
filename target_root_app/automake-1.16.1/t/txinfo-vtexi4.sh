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

# Check that the version.texi file is automatically created and distributed
# if @included into a texi source.  Also check that is correctly defined
# @values definitions it is advertised to.
# See also the related test 'txinfo-vtexi4.sh', which does similar checks,
# but for more vers*.texi files, and does not require makeinfo, tex and
# texi2dvi.

required='makeinfo tex texi2dvi grep-nonprint'
. test-init.sh

test $(LC_ALL=C date '+%u') -gt 0 && test $(LC_ALL=C date '+%u') -lt 8 \
  && day=$(LC_ALL=C date '+%d')   && test -n "$day" \
  && month=$(LC_ALL=C date '+%B') && test -n "$month" \
  && year=$(LC_ALL=C date '+%Y')  && test -n "$year" \
  || skip_ "'date' is not POSIX-compliant enough"
day=$(echo "$day" | sed 's/^0//')

cat > configure.ac << END
AC_INIT([$me], [123.456])
AM_INIT_AUTOMAKE
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
END

cat > defs.am <<END
my_date_rx = $day $month $year
my_month_rx = $month $year
my_version_rx = 123\.456
END

cat > Makefile.am << 'END'
include defs.am
info_TEXINFOS = foo.texi
test-grepinfo:
## Not useless uses of cat: we only tested that grep works on
## non-text input when that's given from a pipe.
	cat $(srcdir)/foo.info | grep 'GREPVERSION=$(my_version_rx)='
	cat $(srcdir)/foo.info | grep 'GREPEDITION=$(my_version_rx)='
	cat $(srcdir)/foo.info | grep 'GREPDATE=$(my_date_rx)='
	cat $(srcdir)/foo.info | grep 'GREPMONTH=$(my_month_rx)='
test-distfiles:
	@echo DISTFILES = $(DISTFILES)
	echo ' ' $(DISTFILES) ' ' | grep '[ /]version.texi '
test-distdir: distdir
	ls -l $(distdir)
	diff $(srcdir)/version.texi $(distdir)/version.texi
.PHONY: test-grepinfo test-distfiles test-distdir
check-local: test-grepinfo test-distfiles test-distdir
END

cat > foo.texi << 'END'
\input texinfo
@c %**start of header
@setfilename foo.info
@settitle Zardoz
@c %**end of header

@node Top
@include version.texi

GREPVERSION=@value{VERSION}=

GREPEDITION=@value{EDITION}=

GREPDATE=@value{UPDATED}=

GREPMONTH=@value{UPDATED-MONTH}=

@bye
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure
$MAKE all dvi

# Debugging & sanity checks.
ls -l
cat version.texi
cat foo.info
test -f foo.dvi

$MAKE test-grepinfo
$MAKE test-distfiles
$MAKE test-distdir
$MAKE distcheck

:
