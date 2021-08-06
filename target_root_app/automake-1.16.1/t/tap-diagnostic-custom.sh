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
#  - option '--diagnostic-string' to customize the string introducing
#    TAP diagnostics

. test-init.sh

fetch_tap_driver

cat >> configure.ac <<END
AC_OUTPUT
END

cat > Makefile.am << 'END'
my_log_driver = $(srcdir)/tap-driver
my_log_compiler = cat
TEST_EXTENSIONS =
TESTS =
END

: > later.mk

# Quoting our comments below is an hack required to keep
# comments near the things they refer to.
i=0
for string in \
'## A letter' \
    a \
'## A number' \
    1023 \
'## A non-alphabetic character' \
    @ \
'## Some metacharacters (we need to repeat the "$" for make)' \
    '^>;&*"|$$' \
'## A whitespace character' \
    " " \
'## A tab character' \
    "$tab" \
'## A string with more whitespace' \
    "  ${tab}a b${tab} c" \
'## Note the we do not have the empty string here.  We prefer to' \
'## leave its behaviour in this context undefined for the moment.'
do
  case $string in '##'*) continue;; esac
  i=$(($i + 1))
  unindent >> Makefile.am << END
    TEST_EXTENSIONS += .t$i
    TESTS += foo$i.t$i
    T${i}_LOG_COMPILER = \$(my_log_compiler)
    T${i}_LOG_DRIVER = \$(my_log_driver)
    AM_T${i}_LOG_DRIVER_FLAGS = \
      --comments \
      --diagnostic-string '$string'
END
  unindent > foo$i.t$i <<END
    1..1
    ok 1
    $string blah blah $i
END
    echo "AM_T${i}_LOG_DRIVER_FLAGS = --no-comments" >> later.mk
done

$ACLOCAL
$AUTOCONF
$AUTOMAKE

./configure

run_make -O check
count_test_results total=$i pass=$i fail=0 xpass=0 xfail=0 skip=0 error=0

cat later.mk >> Makefile
run_make -O check
$FGREP 'blah blah' stdout && exit 1

:
