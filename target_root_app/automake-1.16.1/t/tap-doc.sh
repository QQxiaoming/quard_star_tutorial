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

# Check that an example given in the documentation really works.
# See section "Simple Tests" subsection "Script-based Testsuites".

. test-init.sh

fetch_tap_driver

cat >> configure.ac <<END
AC_PROG_CC
AC_OUTPUT
END

cat > Makefile.am << 'END'
TESTS = foo.sh zardoz.tap bar.sh mu.tap
TEST_EXTENSIONS = .sh .tap
TAP_LOG_DRIVER = $(srcdir)/tap-driver
## Ensure the test scripts are run in the correct order.
mu.log: bar.log
bar.log: zardoz.log
zardoz.log: foo.log
END

cat > foo.sh <<'END'
#!/bin/sh
exit 0
END

cat > bar.sh <<'END'
#!/bin/sh
exit 77
END

cat > zardoz.tap << 'END'
#!/bin/sh
echo 1..4
echo 'ok 1 - Daemon started'
echo 'ok 2 - Daemon responding'
echo 'ok 3 - Daemon uses /proc # SKIP /proc is not mounted'
echo 'ok 4 - Daemon stopped'
END

cat > mu.tap << 'END'
#!/bin/sh
echo 1..2
echo 'ok'
echo 'not ok # TODO frobnication not yet implemented'
END

chmod a+x *.sh *.tap

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure

run_make -O check

cat > exp <<'END'
PASS: foo.sh
PASS: zardoz.tap 1 - Daemon started
PASS: zardoz.tap 2 - Daemon responding
SKIP: zardoz.tap 3 - Daemon uses /proc # SKIP /proc is not mounted
PASS: zardoz.tap 4 - Daemon stopped
SKIP: bar.sh
PASS: mu.tap 1
XFAIL: mu.tap 2 # TODO frobnication not yet implemented
END

sed -n '/^PASS: foo\.sh/,/^XFAIL: mu\.tap/p' stdout > t
cat t
# Strip extra "informative" lines that could be printed by Solaris
# Distributed Make.
LC_ALL=C $EGREP -v ' --> ([0-9][0-9]* job|[Jj]ob output)' t > got

cat exp
cat got
diff exp got

:
