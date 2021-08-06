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

# Check the testsuite summary with the parallel test harness.  This
# script is meant to be sourced by other test script, so that it can
# be used to check different scenarios (colorized and non-colorized
# testsuite output, packages with and without bug-report addresses,
# testsuites in subdirectories, ...)

. test-init.sh

case $use_colors in
  yes)
    AM_COLOR_TESTS=always; export AM_COLOR_TESTS
    # Forced colorization should take place also with non-ANSI
    # terminals; hence this setting.
    TERM=dumb; export TERM
    ;;
  no)
    ;;
  *)
    fatal_ "invalid use_colors='$use_colors'";;
esac

cat > configure.ac <<END
AC_INIT([GNU AutoFoo], [7.1], [bug-automake@gnu.org])
AM_INIT_AUTOMAKE
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
END

cat > Makefile.am <<'END'
TEST_EXTENSIONS = .t
T_LOG_COMPILER = $(SHELL) $(srcdir)/log-compiler
## Will be defined at runtime.
TESTS =
$(TESTS):
END

cat > log-compiler <<'END'
#!/bin/sh
case "$1" in
  pass*|*/pass*|xpass*|*/xpass*) exit 0;;
  fail*|*/fail*|xfail*|*/xfail*) exit 1;;
  skip*|*/skip*) exit 77;;
  error*|/error*) exit 99;;
  copy*|*/copy*) cat "$1";;
  *) exit 99;;
esac
END

# Quite complexish, but allow the tests in client scripts to be written
# in a "data-driven fashion".
do_check ()
{
  cat > summary.exp
  expect_failure=false
  xfail_tests=''
  tests="$*"
  for t in $*; do
    case $t in fail*|xpass*|error*) expect_failure=:;; esac
    case $t in xfail*|xpass*) xfail_tests="$xfail_tests $t";; esac
  done
  run_make -O -e IGNORE check TESTS="$tests" XFAIL_TESTS="$xfail_tests"
  if $expect_failure; then
    test $am_make_rc -gt 0 || exit 1
  else
    test $am_make_rc -eq 0 || exit 1
  fi
  $PERL "$am_testaux_srcdir"/extract-testsuite-summary.pl stdout >summary.got \
   || fatal_ "cannot extract testsuite summary"
  cat summary.exp
  cat summary.got
  if test $use_colors = yes; then
    # Use cmp, not diff, because the files might contain binary data.
    compare=cmp
  else
    compare=diff
  fi
  $compare summary.exp summary.got || exit 1
}

br='============================================================================'

$ACLOCAL
$AUTOCONF
$AUTOMAKE --add-missing

:
