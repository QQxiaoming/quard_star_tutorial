#! /bin/sh
# Copyright (C) 2005-2018 Free Software Foundation, Inc.
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

# Make sure 'make -k check' processes all directories.

# For gen-testsuite-part: ==> try-with-serial-tests <==
. test-init.sh

cat >> configure.ac << 'END'
AC_CONFIG_FILES([dir/Makefile])
AC_OUTPUT
END

mkdir dir

cat > Makefile.am <<'END'
SUBDIRS = dir
TESTS = ok.sh
END

echo TESTS = fail.sh >dir/Makefile.am

cat >ok.sh <<'END'
#!/bin/sh
:
END

cat >dir/fail.sh <<'END'
#!/bin/sh
exit 1
END

chmod +x ok.sh dir/fail.sh

$ACLOCAL
$AUTOCONF
$AUTOMAKE --add-missing

./configure --prefix "$(pwd)/inst"

run_make -O -e FAIL -- check
grep '^FAIL: fail\.sh *$' stdout
grep '^PASS: ok\.sh *$' stdout && exit 1

# The exit status of 'make -k' can be anything
# (depending on the Make implementation)
run_make -O -e IGNORE -- -k check
grep '^FAIL: fail\.sh *$' stdout
grep '^PASS: ok\.sh *$' stdout

# Should also works when -k is not in first position.
run_make -O -e IGNORE -- -s -k check
grep '^FAIL: fail\.sh *' stdout
grep '^PASS: ok\.sh *' stdout

# The rest of the test is for GNU Make.

if using_gmake; then
  # Try with a long-option that do not have a short option equivalent
  # (here, --no-print-directory).  That should cause all options to
  # appear verbatim in MAKEFLAGS.
  run_make -e FAIL -O -- --no-print-directory -k check
  grep '^FAIL: fail\.sh *$' stdout
  grep '^PASS: ok\.sh *$' stdout
fi

:
