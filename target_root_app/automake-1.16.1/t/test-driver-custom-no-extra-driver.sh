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

# Check that auxiliary script 'test-driver' doesn't get needlessly
# installed or referenced if it's not used, i.e., if the user has
# defined his own '*LOG_DRIVER' variables.

. test-init.sh

cat >> configure.ac <<'END'
AC_PROG_CC
AC_CONFIG_FILES([sub1/Makefile sub2/Makefile])
AC_SUBST([X_LOG_DRIVER], [none])
AC_OUTPUT
END

mkdir sub1 sub2

cat > Makefile.am <<'END'
SUBDIRS = sub1 sub2
LOG_DRIVER =
TEST_LOG_DRIVER = :
TESTS = foo bar.test
END

cat > sub1/Makefile.am <<'END'
TEST_EXTENSIONS = .x .sh .pl
SH_LOG_DRIVER = dummy1
PL_LOG_DRIVER = dummy2
TESTS = a.pl b.sh c.x
END

cat > sub2/Makefile.am <<'END'
TEST_EXTENSIONS = .bar
LOG_DRIVER = x
BAR_LOG_DRIVER = y
TESTS = 1 2.bar 3.test 4.t 5.tt $(check_PROGRAMS)
check_PROGRAMS = p1 p2$(EXEEXT) p3.bar p4.suf
END

$ACLOCAL

for opts in '' '--add-missing' '-a -c'; do
  $AUTOMAKE $opts
  $FGREP test-driver Makefile.in sub[12]/Makefile.in && exit 1
  find . | $FGREP test-driver && exit 1
  : For shells with busted 'set -e'.
done

:
