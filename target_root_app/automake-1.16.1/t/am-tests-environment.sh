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

# parallel-tests: check AM_TESTS_ENVIRONMENT support, and its
# interactions with TESTS_ENVIRONMENT.

. test-init.sh

cat >> configure.ac << 'END'
AC_PROG_MKDIR_P
AC_OUTPUT
END

cat > Makefile.am << 'END'
TEST_EXTENSIONS = .sh .test
TESTS = foo.test bar.sh
SH_LOG_COMPILER = sh
AM_TESTS_ENVIRONMENT = \
  $(MKDIR_P) quux.dir; \
  if test -f $(srcdir)/test-env.sh; then \
    . $(srcdir)/test-env.sh; \
  fi; \
  FOO=1; export FOO;
EXTRA_DIST = $(TESTS) test-env.sh
END

cat > foo.test << 'END'
#! /bin/sh
ls -l && test -d quux.dir
END
chmod a+x foo.test

cat > bar.sh << 'END'
echo "FOO='$FOO'"
echo "BAR='$BAR'"
test x"$FOO" = x"$BAR"
END

debug_info ()
{
  cat test-suite.log
  cat foo.log
  cat bar.log
}

minicheck ()
{
  debug_info
  test -d quux.dir
}

miniclean ()
{
  rmdir quux.dir
  rm -f foo.log bar.log test-suite.log
}

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure

TESTS_ENVIRONMENT='BAR=1' $MAKE check || { debug_info; exit 1; }
minicheck
miniclean

TESTS_ENVIRONMENT='BAR=2' $MAKE check && { debug_info; exit 1; }
minicheck
miniclean

echo 'BAR=1 && export BAR' > test-env.sh
$MAKE check || { debug_info; exit 1; }
minicheck
$MAKE distcheck

:
