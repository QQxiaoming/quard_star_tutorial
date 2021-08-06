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

# Auxiliary script for tests on TAP support: checking testsuite summary.

. test-init.sh

br='============================================================================'

case $use_colors in
  yes|no) ;;
  *) fatal_ "invalid \$use_colors value '$use_colors'"
esac

fetch_tap_driver

cat > configure.ac <<END
AC_INIT([GNU AutoTAP], [5.12], [bug-automake@gnu.org])
AM_INIT_AUTOMAKE
AC_CONFIG_FILES([Makefile])
AC_OUTPUT
END

cat > Makefile.am << 'END'
TEST_LOG_DRIVER = $(srcdir)/tap-driver
TEST_LOG_COMPILER = cat
TESTS = all.test
END

# The following shell variables are influential for this function:
#  - expect_failure
#  - use_colors
do_check ()
{
  case $#,$1 in
    1,--pass) expect_failure=no;;
    1,--fail) expect_failure=yes;;
           *) fatal_ "invalid usage of 'do_check'";;
  esac
  shift
  cat > summary.exp
  cat all.test
  if test $use_colors = yes; then
    # Forced colorization should take place also with non-ANSI terminals;
    # hence the "TERM=dumb" definition.
    make_args='TERM=dumb AM_COLOR_TESTS=always'
  else
    make_args=
  fi
  run_make -O -e IGNORE $make_args check
  if test $expect_failure = yes; then
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

if test $use_colors = yes; then
  red="$esc[0;31m"
  grn="$esc[0;32m"
  lgn="$esc[1;32m"
  blu="$esc[1;34m"
  mgn="$esc[0;35m"
  brg="$esc[1m"
  std="$esc[m"
else
  red= grn= lgn= blu= mgn= brg= std=
fi

  success_header="\
${grn}${br}${std}
${grn}Testsuite summary for GNU AutoTAP 5.12${std}
${grn}${br}${std}"

  success_footer=${grn}${br}${std}

  failure_header="\
${red}${br}${std}
${red}Testsuite summary for GNU AutoTAP 5.12${std}
${red}${br}${std}"

  failure_footer="\
${red}${br}${std}
${red}See ./test-suite.log${std}
${red}Please report to bug-automake@gnu.org${std}
${red}${br}${std}"

$ACLOCAL
$AUTOCONF
$AUTOMAKE --add-missing

./configure

# 1 pass.
{ echo 1..1 && echo ok; } > all.test
do_check --pass <<END
$success_header
${brg}# TOTAL: 1${std}
${grn}# PASS:  1${std}
# SKIP:  0
# XFAIL: 0
# FAIL:  0
# XPASS: 0
# ERROR: 0
$success_footer
END

# 1 skip.
{ echo 1..1 && echo 'ok # SKIP'; } > all.test
do_check --pass <<END
$success_header
${brg}# TOTAL: 1${std}
# PASS:  0
${blu}# SKIP:  1${std}
# XFAIL: 0
# FAIL:  0
# XPASS: 0
# ERROR: 0
$success_footer
END

# 1 xfail.
{ echo 1..1 && echo 'not ok # TODO'; } > all.test
do_check --pass <<END
$success_header
${brg}# TOTAL: 1${std}
# PASS:  0
# SKIP:  0
${lgn}# XFAIL: 1${std}
# FAIL:  0
# XPASS: 0
# ERROR: 0
$success_footer
END

# 1 fail.
{ echo 1..1 && echo not ok; } > all.test
do_check --fail <<END
$failure_header
${brg}# TOTAL: 1${std}
# PASS:  0
# SKIP:  0
# XFAIL: 0
${red}# FAIL:  1${std}
# XPASS: 0
# ERROR: 0
$failure_footer
END

# 1 xpass.
{ echo 1..1 && echo 'ok # TODO'; } > all.test
do_check --fail <<END
$failure_header
${brg}# TOTAL: 1${std}
# PASS:  0
# SKIP:  0
# XFAIL: 0
# FAIL:  0
${red}# XPASS: 1${std}
# ERROR: 0
$failure_footer
END

# 1 hard error.
{ echo 1..1 && echo 'Bail out!'; } > all.test
do_check --fail <<END
$failure_header
${brg}# TOTAL: 1${std}
# PASS:  0
# SKIP:  0
# XFAIL: 0
# FAIL:  0
# XPASS: 0
${mgn}# ERROR: 1${std}
$failure_footer
END

# 3 non-failing results.
cat > all.test <<END
1..3
ok
not ok # TODO
ok # SKIP
END
do_check --pass <<END
$success_header
${brg}# TOTAL: 3${std}
${grn}# PASS:  1${std}
${blu}# SKIP:  1${std}
${lgn}# XFAIL: 1${std}
# FAIL:  0
# XPASS: 0
# ERROR: 0
$success_footer
END

# 1 pass, 1 skip, 1 fail.
cat > all.test <<END
1..3
ok
ok # SKIP
not ok
END
do_check --fail <<END
$failure_header
${brg}# TOTAL: 3${std}
${grn}# PASS:  1${std}
${blu}# SKIP:  1${std}
# XFAIL: 0
${red}# FAIL:  1${std}
# XPASS: 0
# ERROR: 0
$failure_footer
END

# 1 pass, 1 xfail, 1 xpass.
cat > all.test <<END
1..3
ok
ok # TODO
not ok # TODO
END
do_check --fail <<END
$failure_header
${brg}# TOTAL: 3${std}
${grn}# PASS:  1${std}
# SKIP:  0
${lgn}# XFAIL: 1${std}
# FAIL:  0
${red}# XPASS: 1${std}
# ERROR: 0
$failure_footer
END

# 1 skip, 1 xfail, 1 error.
cat > all.test <<END
1..3
ok # SKIP
not ok # TODO
Bail out!
END
do_check --fail <<END
$failure_header
${brg}# TOTAL: 3${std}
# PASS:  0
${blu}# SKIP:  1${std}
${lgn}# XFAIL: 1${std}
# FAIL:  0
# XPASS: 0
${mgn}# ERROR: 1${std}
$failure_footer
END

# 1 of each kind
cat > all.test <<END
1..6
ok
not ok
ok # TODO
not ok # TODO
ok # SKIP
Bail out!
END
do_check --fail <<END
$failure_header
${brg}# TOTAL: 6${std}
${grn}# PASS:  1${std}
${blu}# SKIP:  1${std}
${lgn}# XFAIL: 1${std}
${red}# FAIL:  1${std}
${red}# XPASS: 1${std}
${mgn}# ERROR: 1${std}
$failure_footer
END

# Prepare some common data for later.
for i in 0 1 2 3 4 5 6 7 8 9; do
  for j in 0 1 2 3 4 5 6 7 8 9; do
    echo "ok"
    echo "not ok # TODO"
    echo "ok # SKIP"
  done
done > tap

# Lots of non-failures (300 per kind).
(cat tap && cat tap && cat tap) > all.test
test $(wc -l <all.test) -eq 900 || exit 99 # Sanity check.
echo 1..900 >> all.test # Test plan.
do_check --pass <<END
$success_header
${brg}# TOTAL: 900${std}
${grn}# PASS:  300${std}
${blu}# SKIP:  300${std}
${lgn}# XFAIL: 300${std}
# FAIL:  0
# XPASS: 0
# ERROR: 0
$success_footer
END

# 1 failure and lots of non-failures means failure.
(cat tap && echo "not ok" && cat tap) > all.test
test $(wc -l <all.test) -eq 601 || exit 99 # Sanity check.
echo 1..601 >> all.test # Test plan.
do_check --fail <<END
$failure_header
${brg}# TOTAL: 601${std}
${grn}# PASS:  200${std}
${blu}# SKIP:  200${std}
${lgn}# XFAIL: 200${std}
${red}# FAIL:  1${std}
# XPASS: 0
# ERROR: 0
$failure_footer
END

# 1 error and lots of non-failures means failure.
(cat tap && sed 30q tap && echo 'Bail out!') > all.test
test $(wc -l <all.test) -eq 331 || exit 99 # Sanity check.
echo 1..331 >> all.test # Test plan.
do_check --fail <<END
$failure_header
${brg}# TOTAL: 331${std}
${grn}# PASS:  110${std}
${blu}# SKIP:  110${std}
${lgn}# XFAIL: 110${std}
# FAIL:  0
# XPASS: 0
${mgn}# ERROR: 1${std}
$failure_footer
END

:
