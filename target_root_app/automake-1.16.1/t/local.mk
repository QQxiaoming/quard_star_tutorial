## -*- makefile-automake -*-
## Copyright (C) 1995-2018 Free Software Foundation, Inc.
##
## This program is free software; you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation; either version 2, or (at your option)
## any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program.  If not, see <https://www.gnu.org/licenses/>.

## ------------ ##
##  Testsuite.  ##
## ------------ ##

# Run the tests with a proper shell detected at configure time.
LOG_COMPILER = ./pre-inst-env $(AM_TEST_RUNNER_SHELL)

TEST_EXTENSIONS = .pl .sh .tap
SH_LOG_COMPILER = $(LOG_COMPILER)
TAP_LOG_COMPILER = $(LOG_COMPILER)
PL_LOG_COMPILER = ./pre-inst-env $(PERL)
AM_PL_LOG_FLAGS = -Mstrict -w

TAP_LOG_DRIVER = AM_TAP_AWK='$(AWK)' $(SHELL) $(srcdir)/lib/tap-driver.sh

AM_TAP_LOG_DRIVER_FLAGS = --merge

EXTRA_DIST += %D%/README %D%/ax/is %D%/ax/is_newest %D%/ax/deltree.pl


## Will be updated later.
TESTS =

# Some testsuite-influential variables should be overridable from the
# test scripts, but not from the environment.
# Keep this in sync with the similar list in ax/runtest.in.
AM_TESTS_ENVIRONMENT = \
  for v in \
    required \
    am_test_protocol \
    am_serial_tests \
    am_test_prefer_config_shell \
    am_original_AUTOMAKE \
    am_original_ACLOCAL \
    am_test_lib_sourced \
    test_lib_sourced \
  ; do \
    eval test x"\$${$$v}" = x || unset $$v; \
  done;
# We want warning messages and explanations for skipped tests to go to
# the console if possible, so set up 'stderr_fileno_' properly.
AM_TESTS_FD_REDIRECT = 9>&2
AM_TESTS_ENVIRONMENT += stderr_fileno_=9; export stderr_fileno_;

# For sourcing of extra "shell libraries" by our test scripts.  As per
# POSIX, sourcing a file with '.' will cause it to be looked up in $PATH
# in case it is given with a relative name containing no slashes.
AM_TESTS_ENVIRONMENT += \
  if test $(srcdir) != .; then \
    PATH='$(abs_srcdir)/%D%/ax$(PATH_SEPARATOR)'$$PATH; \
  fi; \
  PATH='$(abs_builddir)/%D%/ax$(PATH_SEPARATOR)'$$PATH; \
  export PATH;

# Hand-written tests.

include $(srcdir)/%D%/list-of-tests.mk

TESTS += $(handwritten_TESTS)
EXTRA_DIST += $(handwritten_TESTS)

# Automatically-generated tests wrapping hand-written ones.
# Also, automatically-computed dependencies for tests.

include $(srcdir)/%D%/testsuite-part.am

TESTS += $(generated_TESTS)
EXTRA_DIST += $(generated_TESTS)

$(srcdir)/%D%/testsuite-part.am:
	$(AM_V_at)rm -f %D%/testsuite-part.tmp $@
	$(AM_V_GEN)$(PERL) $(srcdir)/gen-testsuite-part \
	  --srcdir $(srcdir) > %D%/testsuite-part.tmp
	$(AM_V_at)chmod a-w %D%/testsuite-part.tmp
	$(AM_V_at)mv -f %D%/testsuite-part.tmp $@
EXTRA_DIST += gen-testsuite-part

# The dependecies declared here are not truly complete, but such
# completeness would cause more issues than it would solve.  See
# automake bug#11347.
$(generated_TESTS): $(srcdir)/gen-testsuite-part
$(srcdir)/%D%/testsuite-part.am: $(srcdir)/gen-testsuite-part
$(srcdir)/%D%/testsuite-part.am: Makefile.am

# Hand-written tests for stuff in 'contrib/'.
include $(srcdir)/contrib/%D%/local.mk
TESTS += $(contrib_TESTS)
EXTRA_DIST += $(contrib_TESTS)

# Static dependencies valid for each test case (also further
# extended later).  Note that use 'noinst_' rather than 'check_'
# as the prefix, because we really want them to be built by
# "make all".  This makes it easier to run the test cases by
# hand after having simply configured and built the package.

dist_noinst_DATA += \
  %D%/ax/test-init.sh \
  %D%/ax/test-lib.sh \
  %D%/ax/am-test-lib.sh \
  %D%/ax/tap-functions.sh

# Few more static dependencies.
%D%/distcheck-missing-m4.log: %D%/ax/distcheck-hook-m4.am
%D%/distcheck-outdated-m4.log: %D%/ax/distcheck-hook-m4.am
EXTRA_DIST += %D%/ax/distcheck-hook-m4.am

%D%/ax/test-defs.sh: %D%/ax/test-defs.in Makefile
	$(AM_V_at)rm -f $@ $@-t
	$(AM_V_at)$(MKDIR_P) %D%/ax
	$(AM_V_GEN)in=%D%/ax/test-defs.in \
	  && $(do_subst) <$(srcdir)/$$in >$@-t
	$(generated_file_finalize)
EXTRA_DIST += %D%/ax/test-defs.in
CLEANFILES += %D%/ax/test-defs.sh
nodist_noinst_DATA += %D%/ax/test-defs.sh

%D%/ax/shell-no-trail-bslash: %D%/ax/shell-no-trail-bslash.in Makefile
	$(AM_V_at)rm -f $@ $@-t
	$(AM_V_GEN)in=%D%/ax/shell-no-trail-bslash.in \
	  && $(MKDIR_P) %D%/ax \
	  && $(do_subst) <$(srcdir)/$$in >$@-t \
	  && chmod a+x $@-t
	$(generated_file_finalize)
EXTRA_DIST += %D%/ax/shell-no-trail-bslash.in
CLEANFILES += %D%/ax/shell-no-trail-bslash
nodist_noinst_SCRIPTS += %D%/ax/shell-no-trail-bslash

%D%/ax/cc-no-c-o: %D%/ax/cc-no-c-o.in Makefile
	$(AM_V_at)rm -f $@ $@-t
	$(AM_V_GEN)in=%D%/ax/cc-no-c-o.in \
	  && $(MKDIR_P) %D%/ax \
	  && $(do_subst) <$(srcdir)/$$in >$@-t \
	  && chmod a+x $@-t
	$(generated_file_finalize)
EXTRA_DIST += %D%/ax/cc-no-c-o.in
CLEANFILES += %D%/ax/cc-no-c-o
nodist_noinst_SCRIPTS += %D%/ax/cc-no-c-o

runtest: %D%/ax/runtest.in Makefile
	$(AM_V_at)rm -f $@ $@-t
	$(AM_V_GEN)in=%D%/ax/runtest.in \
	  && $(MKDIR_P) %D%/ax \
	  && $(do_subst) <$(srcdir)/$$in >$@-t \
	  && chmod a+x $@-t
	$(generated_file_finalize)
EXTRA_DIST += %D%/ax/runtest.in
CLEANFILES += runtest
nodist_noinst_SCRIPTS += runtest

# If two test scripts have the same basename, they will end up sharing
# the same log file, leading to all sort of undefined and undesired
# behaviours.
check-no-repeated-test-name:
	@LC_ALL=C; export LC_ALL; \
	 lst='$(TEST_LOGS)'; for log in $$lst; do echo $$log; done \
	   | sort | uniq -c | awk '($$1 > 1) { print }' \
	   | sed 's/\.log$$//' | grep . >&2 \
	   && { \
	     echo $@: test names listed above are duplicated >&2; \
	     exit 1; \
	   }; :
check-local: check-no-repeated-test-name
.PHONY: check-no-repeated-test-name

# Check that our test cases are syntactically correct.
# See automake bug#11898.
check-tests-syntax:
	@st=0; \
	err () { echo "$@: $$*" >&2; st=1; }; \
## The user might do something like "make check TESTS=t/foo" or
## "make check TESTS_LOGS=t/foo.log" and expect (say) the test
## 't/foo.sh' to be run; this has worked well until today, and
## we want to continue supporting this use case.
	bases=`for log in : $(TEST_LOGS); do echo $$log; done \
	  | sed -e '/^:$$/d' -e 's/\.log$$//'`; \
	for bas in $$bases; do \
	  for suf in sh tap pl; do \
	    tst=$$bas.$$suf; \
## Emulate VPATH search.
	    if test -f $$tst; then \
	      break; \
	    elif test -f $(srcdir)/$$tst; then \
	      tst=$(srcdir)/$$tst; \
	      break; \
	    else \
	      tst=''; \
	    fi; \
	  done; \
	  test -n "$$tst" || err "couldn't find test '$$bas'"; \
## Don't check that perl tests are valid shell scripts!
	  test $$suf = pl && continue; \
	  $(AM_V_P) && echo " $(AM_TEST_RUNNER_SHELL) -n $$tst"; \
	  $(AM_TEST_RUNNER_SHELL) -n "$$tst" \
	    || err "test '$$tst' syntactically invalid"; \
	done; \
	exit $$st
check-local: check-tests-syntax
.PHONY: check-tests-syntax

# Recipes with a trailing backslash character (possibly followed by
# blank characters only) can cause spurious syntax errors with at
# least older bash versions (e.g., bash 2.05b), and can be potentially
# be unportable to other weaker shells.  Run the testsuite in a way
# that helps catching such problems in Automake-generated recipes.
# See automake bug#10436.
check-no-trailing-backslash-in-recipes:
	$(AM_V_GEN)$(MAKE) $(AM_MAKEFLAGS) check \
	  CONFIG_SHELL='$(abs_top_builddir)/%D%/ax/shell-no-trail-bslash'
.PHONY: check-no-trailing-backslash-in-recipes

# Some compilers out there (hello, MSVC) still choke on "-c -o" being
# passed together on the command line.  Run the whole testsuite faking
# the presence of such a compiler, to help catch regressions that would
# otherwise only present themselves later "in the wild".  See also the
# long discussion about automake bug#13378.
check-cc-no-c-o:
	$(AM_V_GEN)$(MAKE) $(AM_MAKEFLAGS) check \
	  AM_TESTSUITE_SIMULATING_NO_CC_C_O=yes
.PHONY: check-cc-no-c-o

# Automake-generated Makefiles should work when run with parallel make.
check-parallel:
	$(AM_V_GEN)$(MAKE) $(AM_MAKEFLAGS) check \
	  AM_TESTSUITE_MAKE="$${AM_TESTSUITE_MAKE-$${MAKE-make}} -j4"
.PHONY: check-parallel

## Checking the list of tests.
test_subdirs = %D% %D%/pm contrib/%D%
include %D%/CheckListOfTests.am

# Run the testsuite with the installed aclocal and automake.
installcheck-local: installcheck-testsuite
installcheck-testsuite:
	$(AM_V_GEN)$(MAKE) $(AM_MAKEFLAGS) check \
	  am_running_installcheck=yes

# Performance tests.
.PHONY: perf
perf: all
	$(AM_V_GEN)$(MAKE) $(AM_MAKEFLAGS) check \
	  TEST_SUITE_LOG='$(PERF_TEST_SUITE_LOG)' TESTS='$(perf_TESTS)'
PERF_TEST_SUITE_LOG = %D%/perf/test-suite.log
CLEANFILES += $(PERF_TEST_SUITE_LOG)
EXTRA_DIST += $(perf_TESTS)

clean-local: clean-local-check
.PHONY: clean-local-check
clean-local-check:
	$(AM_V_GEN)$(PERL) $(srcdir)/t/ax/deltree.pl t/*.dir t/*/*.dir */t/*.dir

# vim: ft=automake noet
