# -*- shell-script -*-
#
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

# Helper functions used by TAP-producing tests of the Automake testsuite.

#
# IMPORTANT: All the functions defined in this file can *not* be used
#            from within a subshell, unless explicitly noted otherwise.
#

# The counts of the TAP test results seen so far: total count and
# per-result counts.
tap_count_=0
tap_pass_count_=0
tap_skip_count_=0
tap_fail_count_=0
tap_xfail_count_=0
tap_xpass_count_=0

# not COMMAND [ARGS...]
# ---------------------
# Run the given command and invert its exit status.
not () { ! "$@"; }

# plan_ [unknown|later|lazy|now|NUMBER-OF-PLANNED-TESTS]
# ------------------------------------------------------
# Print a TAP plan for the given number of tests.  This must be called
# before reporting any test result.  If called with the special argument
# "unknown" or "later", it will do nothing, expecting the calling script
# to declare the plan later.  If called with the special argument "lazy"
# or "now", it will print a TAP plan that accounts for the number of tests
# seen so far.
plan_ ()
{
  if test $# -eq 0; then
    bailout_ "plan_: missing argument"
  elif test $# -ge 2; then
    bailout_ "plan_: too many arguments"
  elif test x"$planned_" != x"none" && test x"$planned_" != x"later"; then
    bailout_ "plan_: called to many times"
  elif test x"$1" = x"unknown" || test x"$1" = x"later"; then
    # This means we want to get back later to declaring the TAP plan.
    planned_=later
    return 0
  elif test x"$1" = x"lazy" || test x"$1" = x"now"; then
    planned_=$tap_count_ # Number of test results seen so far.
  elif test $1 -ge 0; then
    planned_=$1
  else
    bailout_ "plan_: invalid argument '$1'"
  fi
  echo "1..$planned_"
}
planned_=none

# diag_ [EXPLANATION]
# ------------------
# Report the given text as TAP diagnostic.  Assumes the string denoting
# TAP diagnostic lines is stored in the '$diag_string_' variable; this is
# done to allow better interplay with TAP drivers that allow such a string
# to be configured.
diag_ ()
{
  test $# -eq 0 || echo "$diag_string_ $*"
}

# Used by the 'diag_' function above.  User-overridable.
diag_string_="#"

# warn_ [EXPLANATION]
# ------------------
# Give a warning (using TAP diagnostic).
warn_ ()
{
  case $# in
    0) diag_ "WARNING: (unknown warning)";;
    *) diag_ "WARNING: $*";;
  esac
}

# result_ RESULT [-D DIRECTIVE] [-r REASON] [--] [DESCRIPTION...]
# ---------------------------------------------------------------
# Report a test case with the given RESULT (valid values are "ok" and
# "not ok") and the given DESCRIPTION (if any).  If DIRECTIVE is given
# and non-empty (valid values being "TODO" and "SKIP"), it will be
# reported too, with the REASON (if given) appended.
result_ ()
{
  set +x # Don't pollute the log files.
  test $# -gt 0 || bailout_ "result_: missing argument"
  tap_result_=$1; shift
  case $tap_result_ in
    "ok"|"not ok") ;;
    *) bailout_ "result_: invalid result '$tap_result'" ;;
  esac
  tap_directive_= tap_reason_=
  while test $# -gt 0; do
    case $1 in
      -D|--directive) tap_directive_=$2; shift;;
      -r|--reason) tap_reason_=$2; shift;;
      --) shift; break;;
      -*) bailout_ "result_: invalid option '$1'";;
       *) break;;
    esac
    shift
  done
  case $tap_directive_ in
    ""|TODO|SKIP) ;;
    *) bailout_ "result_: invalid directive '$directive_'" ;;
  esac
  tap_count_=$(($tap_count_ + 1))
  case $tap_result_,$tap_directive_ in
    ok,)                                                # Passed.
      tap_pass_count_=$(($tap_pass_count_ + 1))         ;;
    not\ ok,TODO)                                       # Expected failure.
      tap_xfail_count_=$(($tap_xfail_count_ + 1))       ;;
    not\ ok,*)                                          # Failed.
      tap_fail_count_=$(($tap_fail_count_ + 1))         ;;
    ok,TODO)                                            # Unexpected pass.
      tap_xpass_count_=$(($tap_xpass_count_ + 1))       ;;
    ok,SKIP)                                            # Skipped.
      tap_skip_count_=$(($tap_skip_count_ + 1))         ;;
    *)                                                  # Can't happen.
      bailout_ "internal error in 'result_'"            ;;
  esac
  tap_text_="$tap_result_ $tap_count_"
  if test x"$*" != x; then
    tap_text_="$tap_text_ - $*"
  fi
  if test x"$tap_directive_" != x; then
    tap_text_="$tap_text_ # $tap_directive_"${tap_reason_:+" $tap_reason_"}
  fi
  printf '%s\n' "$tap_text_"
  set -x # Restore shell xtraces.
}

#  Shorthands for common usages of 'result_'.
ok_ () { result_ 'ok' ${1+"$@"}; }
not_ok_ () { result_ 'not ok' ${1+"$@"}; }
skip_ () { result_ 'ok' -D SKIP ${1+"$@"}; }

# skip_row_ COUNT [-r REASON] [--] [DESCRIPTION...]
# -------------------------------------------------
# Report a COUNT of skipped test, with the given reason and descriptions
# (if any).  Useful to avoid cascade failures in case a fair number of
# tests depend on an earlier one that failed.
skip_row_ ()
{
  skip_count_=$1; shift
  for i_ in $(seq_ $skip_count_); do skip_ ${1+"$@"}; done
}

# skip_all_ [REASON ...]
# ----------------------
# Skip all the tests in a test script.  Must be used before calling 'plan_'
# or reporting any test result.  Can't be used from within a subshell.
skip_all_ ()
{
  echo "1..0 # SKIP" ${1+"$@"}
  planned_=0
  exit 0
}

# bailout_ [REASON ...]
# ---------------------
# Stop the execution of the current test suite right now, due to an
# unrecoverable error.  Can be called at any point, but cannot be used
# from within a subshell.
bailout_ ()
{
  echo 'Bail out!' ${1+"$@"}
  exit 99
}

# fatal_ [REASON ...]
# -------------------
# Same as 'bailout_'; for compatibility with 'plain-functions.sh'.
fatal_ ()
{
  bailout_ ${1+"$@"}
}

# framework_failure_ [REASON ...]
# -------------------------------
# Stop the execution of the current test suite right now, due to an
# unrecoverable error in the set-up of the test case.  Can be called
# at any point, but cannot be used from within a subshell.
framework_failure_ ()
{
  bailout_ "set-up failure"${1+": $*"}
}

# command_ok_ TEST-DESCRIPTION [OPTIONS..] [--] CMD [ARGS...]
# -----------------------------------------------------------
# Helper subroutine for when a TAP result must be determined by the
# outcome of a command.
command_ok_ ()
{
  tap_directive_= tap_reason_=
  test $# -gt 0 || bailout_ "command_ok_: missing argument"
  tap_description_=$1; shift
  while test $# -gt 0; do
    case $1 in
      -D|--directive) tap_directive_=$2; shift;;
      -r|--reason) tap_reason_=$2; shift;;
      --) shift; break;;
      -*) bailout_ "command_ok_: invalid option '$1'";;
       *) break;;
    esac
    shift
  done
  tap_result_="ok"; "$@" || tap_result_="not ok"
  result_ "$tap_result_" -D "$tap_directive_" -r "$tap_reason_" \
          -- "$tap_description_"
}

:
