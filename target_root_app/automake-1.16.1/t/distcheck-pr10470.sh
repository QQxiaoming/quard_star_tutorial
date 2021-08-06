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

# Ensure "make distcheck" does not experience racy failures on
# systems (like MinGW/MSYS) that cannot remove a directory "in use"
# by a process (e.g., that is its "current working directory").
# See automake bug#10470.

. test-init.sh

mkdir foo.d
sh -c "cd foo.d && sleep '4'" &
# Without this sleep, the "rm -rf foo.d" below would reliably beat
# the "cd foo.d" in the subshell above, and the test would be always
# skipped, even on MinGW/MSYS.
sleep '1'
rm -rf foo.d && skip_ 'system is able to remove "in use" directories'

echo AC_OUTPUT >> configure.ac

cat > Makefile.am <<END
TESTS = foo.test
EXTRA_DIST= foo.test
END

cat > foo.test <<END
#!/bin/sh
sleep '4' &
exit 0
END
chmod a+x foo.test

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a
./configure

# We can build the distribution.
run_make -M distcheck
# Sanity check: verify that our code has hit a problem removing
# the distdir, but has recovered from it.
$EGREP "(^| )(rm|find):.*$distdir" output || fatal_ "expected code path not covered"

:
