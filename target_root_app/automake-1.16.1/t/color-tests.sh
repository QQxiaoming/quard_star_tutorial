#! /bin/sh
# Copyright (C) 2007-2018 Free Software Foundation, Inc.
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

# Test Automake TESTS color output, by forcing it.
# Keep this in sync with the sister test 'color-tests2.sh'.

required='grep-nonprint'
# For gen-testsuite-part: ==> try-with-serial-tests <==
. test-init.sh

# Escape '[' for grep, below.
red="$esc\[0;31m"
grn="$esc\[0;32m"
lgn="$esc\[1;32m"
blu="$esc\[1;34m"
mgn="$esc\[0;35m"
std="$esc\[m"

cat >>configure.ac <<END
AC_OUTPUT
END

cat >Makefile.am <<'END'
TESTS = $(check_SCRIPTS)
check_SCRIPTS = pass fail skip xpass xfail error
XFAIL_TESTS = xpass xfail
END

cat >pass <<END
#! /bin/sh
exit 0
END

cat >fail <<END
#! /bin/sh
exit 1
END

cat >skip <<END
#! /bin/sh
exit 77
END

cat >error <<END
#! /bin/sh
exit 99
END

cp fail xfail
cp pass xpass
chmod +x pass fail skip xpass xfail error

$ACLOCAL
$AUTOCONF
$AUTOMAKE --add-missing

test_color ()
{
  # Not a useless use of cat; see above comments "grep-nonprinting"
  # requirement in 'test-init.sh'.
  cat stdout | grep "^${grn}PASS${std}: .*pass"
  cat stdout | grep "^${red}FAIL${std}: .*fail"
  cat stdout | grep "^${blu}SKIP${std}: .*skip"
  cat stdout | grep "^${lgn}XFAIL${std}: .*xfail"
  cat stdout | grep "^${red}XPASS${std}: .*xpass"
  # The old serial testsuite driver doesn't distinguish between failures
  # and hard errors.
  if test x"$am_serial_tests" = x"yes"; then
    cat stdout | grep "^${red}FAIL${std}: .*error"
  else
    cat stdout | grep "^${mgn}ERROR${std}: .*error"
  fi
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
    # Note that we also want to check that the testsuite summary is
    # not unduly colorized.
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

for vpath in false :; do

  if $vpath; then
    mkdir build
    cd build
    srcdir=..
  else
    srcdir=.
  fi

  $srcdir/configure

  # Forced colorization should take place also with non-ANSI terminals;
  # hence the "TERM=dumb" definition.
  AM_COLOR_TESTS=always; export AM_COLOR_TESTS
  run_make -e FAIL -O TERM=dumb check
  test_color

  unset AM_COLOR_TESTS
  run_make -e FAIL -O TERM=ansi check
  test_no_color

  $MAKE distclean
  cd $srcdir

done

:
