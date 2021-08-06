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
# it advertises to do, even when in subdirectories.
#
# This behavior might be suboptimal, but it has been in place for quite
# a long time, and it would be risky to change it now.  See also the
# discussion of automake bug#7819:
#  <https://debbugs.gnu.org/cgi/bugreport.cgi?bug=7819>
#
# Keep this test in sync with sister test 'autodist.sh'.

. test-init.sh

cat >> configure.ac <<'END'
AC_CONFIG_FILES([sub/Makefile])
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
SUBDIRS = sub
check-local:
## For debugging.
	@echo DIST_COMMON:
	@for f in $(DIST_COMMON); do echo "  $$f"; done
	@echo DISTDIR:
	@ls -l $(distdir) | sed 's/^/  /'
## Now the checks.
	@for f in $(autodist_list); do \
	  echo "file: sub/$$f"; \
	  test -f $(distdir)/sub/$$f \
	    || { echo $$f: distdir fail >&2; exit 1; }; \
	done
END

mkdir sub

cat > sub/Makefile.am <<'END'
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
## Some filenames might contain dots, but this won't cause spurious
## failures, and "spurious successes" are so unlikely that they're
## not worth worrying about.
	  echo ' ' $(DIST_COMMON) ' ' | grep "[ /]$$f " >/dev/null \
	    || { echo $$f: distcom fail >&2; exit 1; }; \
	done
END

: First try listing the automatically-distributed files in proper
: targets in Makefile.am

echo "MAINTAINERCLEANFILES = $list" > sub/distfiles.am
for f in $list; do echo "$f :; touch $f"; done >> sub/distfiles.am

cat sub/distfiles.am # For debugging.

$AUTOMAKE -a

./configure

$MAKE distdir
autodist_list="$list" $MAKE check

$MAKE maintainer-clean
test ! -e sub/README    # Sanity check.
rm -rf $me-1.0          # Remove $(distdir).

: Now try creating the automatically-distributed files before
: running automake.

: > sub/distfiles.am
for f in $list; do
  echo dummy > sub/$f
done

ls -l # For debugging.

$AUTOMAKE

./configure

$MAKE distdir
autodist_list="$list" $MAKE check

:
