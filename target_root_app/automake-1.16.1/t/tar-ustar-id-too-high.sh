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

# Check that UID or GID too high for the ustar format are correctly
# rwcognized and diagnosed by configure.  See bug#8343 and bug#13588.

. test-init.sh

cat > configure.ac <<END
AC_INIT([$me], [1.0])
AM_INIT_AUTOMAKE([tar-ustar])
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
END

: > Makefile.am

run_configure()
{
  st=0; ./configure ${1+"$@"}  >stdout || st=$?
  cat stdout || exit 1
  test $st -eq 0 || exit 1
}

checked ()
{
  grep "^checking $1\.\.\. $2$" stdout
}

$ACLOCAL
$AUTOCONF
$AUTOMAKE

mkdir bin
cat > bin/id <<'END'
#!/bin/sh -e
case "$*" in
  -u) echo "${am_uid-1000}";;
  -g) echo "${am_gid-1000}";;
   *) echo "id: bad/unexpected usage" >&2; exit 1;;
esac
END
chmod a+x bin/id

PATH=$(pwd)/bin$PATH_SEPARATOR$PATH

# Problematic ID reported in
# <https://bugzilla.redhat.com/show_bug.cgi?id=843376>.
am_uid=16777216; export am_uid
am_gid=1000;     export am_gid
run_configure
checked "whether UID '$am_uid' is supported by ustar format" "no"
checked "whether GID '1000' is supported by ustar format" "yes"
checked "how to create a ustar tar archive" "none"

# Another problematic ID reported in
# <https://bugzilla.redhat.com/show_bug.cgi?id=843376>.
am_uid=1000;     export am_uid
am_gid=17000000; export am_gid
run_configure
checked "whether UID '1000' is supported by ustar format" "yes"
checked "whether GID '$am_gid' is supported by ustar format" "no"
checked "how to create a ustar tar archive" "none"

# The minimal ID that is too big.
two_to_twentyone=$((32 * 32 * 32 * 32 * 2))
# <https://bugzilla.redhat.com/show_bug.cgi?id=843376>.
am_uid=$two_to_twentyone; export am_uid
am_gid=$two_to_twentyone; export am_gid
run_configure
checked "whether UID '$two_to_twentyone' is supported by ustar format" "no"
checked "whether GID '$two_to_twentyone' is supported by ustar format" "no"
checked "how to create a ustar tar archive" "none"

:
