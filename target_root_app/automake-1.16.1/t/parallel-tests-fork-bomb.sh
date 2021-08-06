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

# Check parallel-tests features:
#  - If $(TEST_SUITE_LOG) is in $(TEST_LOGS), we get a diagnosed
#    error, not a make hang or a system freeze.

. test-init.sh

# We don't want localized error messages from make, since we'll have
# to grep them.  See automake bug#11452.
LANG=C LANGUAGE=C LC_ALL=C
export LANG LANGUAGE LC_ALL

# The tricky part of this test is to avoid that make hangs or even
# freezes the system in case infinite recursion (which is the bug we
# are testing against) is encountered.  The following hacky makefile
# should minimize the probability of that happening.
cat > Makefile.am << 'END'
TEST_LOG_COMPILER = true
TESTS =

errmsg = ::OOPS:: Recursion too deep

if IS_GNU_MAKE

 is_too_deep := $(shell test $(MAKELEVEL) -lt 10 && echo no)

## Indenteation here required to avoid confusing Automake.
 ifeq ($(is_too_deep),no)
 else
 $(error $(errmsg), $(MAKELEVEL) levels)
 endif

else !IS_GNU_MAKE

# We use mkdir to detect the level of recursion, since it is easy
# to use and assured to be portably atomical.  Also use an higher
# number than with GNU make above, since the level used here can
# be incremented by tow or more per recursion.
recursion-not-too-deep:
	@ok=no; \
	for i in 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15 16 17 \
	         18 19 20 21 22 23 24 25 26 27 28 29; \
	do \
	  echo " mkdir rec-$$i.d"; \
	  if mkdir rec-$$i.d; then \
	    ok=yes; break; \
	  else :; fi; \
	done; \
	test $$ok = yes || { echo '$(errmsg)' >&2; exit 1; }
.PHONY: recursion-not-too-deep
clean-local:
	rmdir rec-[0-9].d

targets = all check recheck $(TESTS) $(TEST_LOGS) $(TEST_SUITE_LOG)
$(targets): recursion-not-too-deep

# For BSD make.
.BEGIN: recursion-not-too-deep

endif !IS_GNU_MAKE
END

if using_gmake; then
  cond=:
else
  cond=false
fi

cat >> configure.ac << END
AM_CONDITIONAL([IS_GNU_MAKE], [$cond])
AC_OUTPUT
END

# Another helpful idiom to avoid hanging on capable systems.  The subshell
# is needed since 'ulimit' might be a special shell builtin.
if (ulimit -t 8); then ulimit -t 8; fi

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a -Wno-portability

./configure

do_check ()
{
  log=$1; shift
  run_make -M -e IGNORE -- "$@" check
  $FGREP '::OOPS::' output && exit 1 # Possible infinite recursion.
  # Check that at least we don't create a botched global log file.
  test ! -e "$log"
  if using_gmake; then
    grep "[Cc]ircular.*dependency" output | $FGREP "$log"
    test $am_make_rc -gt 0
  else
    # Look for possible error messages about circular dependencies from
    # either make or our own recipes.  At least one such a message must
    # be present.  OTOH, some make implementations (e.g., NetBSD's), while
    # smartly detecting the circular dependency early and diagnosing it,
    # still exit with a successful exit status (yikes!).  So don't check
    # the exit status of non-GNU make, to avoid spurious failures.
    # this case.
    err_seen=no
    for err_rx in \
      'circular.* depend' \
      'depend.* circular' \
      'graph cycle' \
      'infinite (loop|recursion)' \
      'depend.* on itself' \
    ; do
      $EGREP -i "$err_rx" output | $FGREP "$log" || continue
      err_seen=yes
      break
    done
    test $err_seen = yes || exit 1
  fi
}

: > test-suite.test
do_check test-suite.log TESTS=test-suite.test
rm -f *.log *.test

: > 0.test
: > 1.test
: > 2.test
: > 3.test
: > foobar.test
do_check foobar.log TEST_LOGS='0.log 1.log foobar.log 2.log 3.log' \
                    TEST_SUITE_LOG=foobar.log
rm -f *.log *.test

:
