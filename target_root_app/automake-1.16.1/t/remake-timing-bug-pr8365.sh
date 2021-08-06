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

# Test for automake bug#8365, related to Makefile remake rules.
# The bug is due to subtle timestamp issues and limitations in
# make's behaviour, and is very unlikely to be triggered (we have
# to resort to timestamp edit hacks to consistently expose it); in
# any account, it is nigh to impossible to trigger it by running
# make by hand.  Thus, fixing it would not be worth the hassle, but
# we prefer to keep it exposed anyway.

. test-init.sh

# We'll use calls to stat to get debugging information.
if stat /dev/null; then stat=stat; else stat=:; fi

cat >> configure.ac << 'END'
FOOBAR=zardoz
AC_OUTPUT
END

: > Makefile.am

$ACLOCAL
# Run automake *before* autoconf, because we want to ensure that
# Makefile.in is not newer than configure.
$AUTOMAKE
$AUTOCONF

./configure
$MAKE Makefile
# Sanity check.
$EGREP 'FOOBAR|zardoz' Makefile && fatal_ 'unexpected AC_SUBST in Makefile'

echo 'AC_SUBST([FOOBAR])' >> configure.ac

# Modified configure dependencies must have the same timestamp of
# config.status and Makefile in order to trigger the bug.
# We also re-touch config.status, because "touch -r" can truncate
# timestamps on file systems with sub-second resolutions (see the
# autoconf manual).  Finally, we also sleep before touching, to ensure
# that the (possibly truncated) timestamps of config.status etc. are
# strictly newer than the non-truncated configure timestamp.
$stat config.status Makefile configure.ac
$sleep
touch config.status
touch -r config.status config.status Makefile configure.ac
$stat config.status Makefile configure.ac

# Also, the race condition is triggered only when aclocal, automake
# and aclocal run fast enough to keep the timestamp of the generated
# aclocal.m4, Makefile.in and configure equal to the timestamp of
# Makefile & config.status.  To reproduce this race consistently, we
# need the following hackish wrappers.

cat > aclocal-wrap <<END
#!/bin/sh
set -ex
# aclocal shouldn't use our autoconf wrapper when extracting
# the races from configure.ac.
AUTOCONF='$AUTOCONF'; export AUTOCONF
$ACLOCAL "\$@"
touch -r config.status aclocal.m4
$stat aclocal.m4
END

cat > automake-wrap <<END
#!/bin/sh
set -ex
# automake shouldn't use our autoconf wrapper when extracting
# the races from configure.ac.
AUTOCONF='$AUTOCONF'; export AUTOCONF
$AUTOMAKE "\$@"
touch -r config.status Makefile.in
$stat Makefile.in
END

cat > autoconf-wrap <<END
#!/bin/sh
set -ex
$AUTOCONF "\$@"
touch -r config.status configure
$stat configure
END

chmod a+x aclocal-wrap automake-wrap autoconf-wrap

run_make Makefile \
  ACLOCAL=./aclocal-wrap AUTOMAKE=./automake-wrap AUTOCONF=./autoconf-wrap
grep '^FOOBAR =' Makefile.in
grep '^FOOBAR *= *zardoz *$' Makefile

:
