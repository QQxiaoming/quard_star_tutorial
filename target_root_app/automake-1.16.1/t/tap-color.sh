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

# TAP support:
#  - colorization of TAP results and diagnostic messages

required='grep-nonprint'
. test-init.sh

# Escape '[' for grep, below.
red="$esc\[0;31m"
grn="$esc\[0;32m"
lgn="$esc\[1;32m"
blu="$esc\[1;34m"
mgn="$esc\[0;35m"
std="$esc\[m"

cat > Makefile.am << 'END'
AUTOMAKE_OPTIONS = color-tests
AM_TEST_LOG_DRIVER_FLAGS = --comments
TEST_LOG_COMPILER = cat
TESTS = all.test skip.test bail.test badplan.test noplan.test \
        few.test many.test order.test afterlate.test
END

. tap-setup.sh

cat > all.test << 'END'
1..5
ok 1 - foo
# Hi! I shouldn't be colorized!
not ok 2 - bar # TODO td
ok 3 - baz # SKIP sk
not ok 4 - quux
ok 5 - zardoz # TODO
END

cat > skip.test << 'END'
1..0 # SKIP whole script
END

cat > bail.test << 'END'
1..1
ok 1
Bail out!
END

cat > badplan.test << 'END'
1..2
ok 1
1..2
ok 2
END

cat > noplan.test << 'END'
ok 1
END

cat > few.test << 'END'
1..2
ok 1
END

cat > many.test << 'END'
1..1
ok 1
ok 2
END

cat > order.test << 'END'
1..1
ok 5
END

cat > afterlate.test << 'END'
ok 1
1..2
ok 2
END

test_color ()
{
  # Not a useless use of cat; see above comments "grep-nonprinting"
  # requirement in 'test-init.sh'.
  cat stdout | grep "^${grn}PASS${std}: all\.test 1 - foo$"
  cat stdout | grep "^${lgn}XFAIL${std}: all\.test 2 - bar # TODO td$"
  cat stdout | grep "^${blu}SKIP${std}: all\.test 3 - baz # SKIP sk$"
  cat stdout | grep "^${red}FAIL${std}: all\.test 4 - quux$"
  cat stdout | grep "^${red}XPASS${std}: all\.test 5 - zardoz # TODO$"
  cat stdout | grep "^${blu}SKIP${std}: skip\.test - whole script$"
  cat stdout | grep "^${grn}PASS${std}: bail\.test 1$"
  cat stdout | grep "^${mgn}ERROR${std}: bail\.test - Bail out!$"
  cat stdout | grep "^${mgn}ERROR${std}: badplan\.test - multiple test plans$"
  cat stdout | grep "^${mgn}ERROR${std}: noplan\.test - missing test plan$"
  cat stdout | grep "^${mgn}ERROR${std}: few.test - too few tests run (expected 2, got 1)$"
  cat stdout | grep "^${mgn}ERROR${std}: many.test - too many tests run (expected 1, got 2)$"
  cat stdout | grep "^${mgn}ERROR${std}: many.test 2 # UNPLANNED$"
  cat stdout | grep "^${mgn}ERROR${std}: order.test 5 # OUT-OF-ORDER (expecting 1)$"
  cat stdout | grep "^${mgn}ERROR${std}: afterlate\.test 2 # AFTER LATE PLAN$"
  # Diagnostic messages shouldn't be colorized.
  cat stdout | grep "^# all\.test: Hi! I shouldn't be colorized!$"
  :
}

test_no_color ()
{
  # With make implementations that, like Solaris make, in case of errors
  # print the whole failing recipe on standard output, we should content
  # ourselves with a laxer check, to avoid false positives.
  # Keep this in sync with lib/am/check.am:$(am__color_tests).
  if $FGREP '= Xalways; then' stdout; then
    # Extra verbose make, resort to laxer checks.
    # But we also want to check that the testsuite summary is not unduly
    # colorized.
    (
      set +e # In case some grepped regex below isn't matched.
      # Not a useless use of cat; see above comments "grep-nonprinting"
      # requirement in 'test-init.sh'.
      cat stdout | grep "TOTAL.*:"
      cat stdout | grep "PASS.*:"
      cat stdout | grep "FAIL.*:"
      cat stdout | grep "SKIP.*:"
      cat stdout | grep "XFAIL.*:"
      cat stdout | grep "XPASS.*:"
      cat stdout | grep "ERROR.*:"
      cat stdout | grep "^#"
      cat stdout | grep 'test.*expected'
      cat stdout | grep 'test.*not run'
      cat stdout | grep '===='
      cat stdout | grep '[Ss]ee .*test-suite\.log'
      cat stdout | grep '[Tt]estsuite summary'
    ) | grep "$esc" && exit 1
    : For shells with broken 'set -e'
  else
    cat stdout | grep "$esc" && exit 1
    : For shells with broken 'set -e'
  fi
}

# Forced colorization should take place also with non-ANSI terminals;
# hence the "TERM=dumb" definition.
AM_COLOR_TESTS=always; export AM_COLOR_TESTS
run_make -O -e FAIL TERM=dumb check
test_color

unset AM_COLOR_TESTS
run_make -O -e FAIL TERM=ansi check
test_no_color

:
