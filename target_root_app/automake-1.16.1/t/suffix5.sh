#! /bin/sh
# Copyright (C) 2001-2018 Free Software Foundation, Inc.
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

# Test to make sure Automake include libtool objects resulting
# from user-defined implicit rules.
# Based on a report from Arkadiusz Miskiewicz <misiek@pld.ORG.PL>.

required='cc libtool'
. test-init.sh

cat >> configure.ac << 'END'
AC_SUBST([LINK], [:])
AM_PROG_AR
AC_PROG_LIBTOOL
AC_OUTPUT
END

cat > Makefile.am << 'END'
.k.lo:
## Account for VPATH issues on weaker make implementations.
	(echo $< && cat `test -f '$<' || echo $(srcdir)/`$<) > $@

noinst_LTLIBRARIES = libfoo.la
libfoo_la_SOURCES = foo.k

.PHONY: test
test: all
	grep '^=GREP=ME=$$' foo.lo
## Weaker regex to account for VPATH issues.
	grep 'foo\.k$$' foo.lo
check-local: test
END

: > ltmain.sh

for auxscript in ar-lib config.guess config.sub; do
  cp "$am_scriptdir/$auxscript" . \
    || fatal_ "fetching auxiliary script '$auxscript'"
done

$ACLOCAL
$AUTOMAKE
grep '_OBJECTS.*foo\.lo' Makefile.in

$AUTOCONF
./configure

echo '=GREP=ME=' > foo.k

$MAKE test
$MAKE distcheck

:
