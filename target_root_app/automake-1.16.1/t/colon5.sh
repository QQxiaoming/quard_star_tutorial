#! /bin/sh
# Copyright (C) 1998-2018 Free Software Foundation, Inc.
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

# Another multi-":" test, this time from Doug Evans.

. test-init.sh

cat > configure.ac <<END
AC_INIT([$me], [1.0])
AM_INIT_AUTOMAKE
AC_CONFIG_FILES([Makefile:Makefile.in:Makefile.dep])
AC_OUTPUT
END

: > Makefile.dep

cat > Makefile.am <<'END'
.PHONY: test-fs-layout test-grep test-distcommon test-distdir
check-local: test-fs-layout test-grep test-distcommon test-distdir
test-fs-layout:
	test x'$(srcdir)' = '.' || test ! -r Makefile.dep
test-grep:
## The use of $(empty) prevents spurious matches.
	grep '=GrEp$(empty)Me_am=' $(srcdir)/Makefile.in
	grep '=GrEp$(empty)Me_dep=' $(srcdir)/Makefile.dep
	grep '=GrEp$(empty)Me_am=' Makefile
	grep '=GrEp$(empty)Me_dep=' Makefile
test-distcommon:
	echo ' ' $(DIST_COMMON) ' ' | grep '[ /]Makefile.dep '
test-distdir: distdir
	test -f $(distdir)/Makefile.dep
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE

./configure

grep '=GrEpMe_am=' Makefile && exit 1  # Sanity check.
grep '=GrEpMe_dep=' Makefile && exit 1 # Likewise.

$MAKE test-distcommon
$MAKE test-distdir

$sleep

echo '# =GrEpMe_am=' >> Makefile.am
echo '# =GrEpMe_dep=' >> Makefile.dep

$MAKE Makefile # For non-GNU make.
$MAKE test-grep
$MAKE test-distcommon
$MAKE test-distdir

$MAKE distcheck

:
