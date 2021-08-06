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

# Check that automake really automatically distributes all the files
# it advertises to do.
# Related to automake bug#7819.
# Keep this test in sync with sister test 'autodist-subdir.sh'.

am_create_testdir=empty
. test-init.sh

cat > configure.ac <<END
AC_INIT([$me], [1.0])
AC_CONFIG_AUX_DIR([.])
AM_INIT_AUTOMAKE
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
END

$ACLOCAL
$AUTOCONF

# The automake manual states that the list of automatically-distributed
# files should be given by 'automake --help'.
list=$($AUTOMAKE --help \
         | sed -n '/^Files.*automatically distributed.*if found.*always/,/^ *$/p' \
         | sed 1d)
# Normalize whitespace, just in case.
list=$(echo $list)

test -n "$list"

cat > Makefile.am <<'END'
include distfiles.am
check-local:
## For debugging.
	@echo DIST_COMMON:
	@for f in $(DIST_COMMON); do echo "  $$f"; done
	@echo DISTDIR:
	@ls -l $(distdir) | sed 's/^/  /'
## Now the checks.
	@for f in $(autodist_list); do \
	  echo "file: $$f"; \
	  test -f $(distdir)/$$f \
	    || { echo $$f: distdir fail >&2; exit 1; }; \
## Some filenames might contain dots, but this won't cause spurious
## failures, and "spurious successes" are so unlikely that they're
## not worth worrying about.
	  echo ' ' $(DIST_COMMON) ' ' | grep "[ /]$$f " >/dev/null \
	    || { echo $$f: distcom fail >&2; exit 1; }; \
	done
END

: First try listing the automatically-distributed files in proper
: targets in Makefile.am

echo "MAINTAINERCLEANFILES = $list" > distfiles.am
for f in $list; do echo "$f :; touch $f"; done >> distfiles.am

cat distfiles.am # For debugging.

$AUTOMAKE -a

./configure

$MAKE distdir
autodist_list="$list" $MAKE check

$MAKE maintainer-clean
test ! -e README        # Sanity check.
rm -rf $me-1.0          # Remove $(distdir).

: Now try creating the automatically-distributed files before
: running automake.

: > distfiles.am
for f in $list; do
  echo dummy > $f
done

ls -l # For debugging.

$AUTOMAKE

./configure

$MAKE distdir
autodist_list="$list" $MAKE check

:
