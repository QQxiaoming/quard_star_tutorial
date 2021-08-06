# -*- shell-script -*-
#
# Copyright (C) 1996-2018 Free Software Foundation, Inc.
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

########################################################
###  IMPORTANT NOTE: keep this file 'set -e' clean.  ###
########################################################

# Do not source several times.
test ${test_lib_sourced-no} = yes && return 0
test_lib_sourced=yes

# CDPATH is evil if used in non-interactive scripts (and even more
# evil if exported in the environment).
CDPATH=; unset CDPATH

# Be more Bourne compatible.
# (Snippet inspired to configure's initialization in Autoconf 2.64)
DUALCASE=1; export DUALCASE # for MKS sh
if test -n "${ZSH_VERSION+set}" && (emulate sh) >/dev/null 2>&1; then
  emulate sh
  NULLCMD=:
  setopt NO_GLOB_SUBST
  # If Zsh is not started directly in POSIX-compatibility mode, it has some
  # incompatibilities in the handling of $0 that conflict with our usage;
  # i.e., $0 inside a file sourced with the '.' builtin is temporarily set
  # to the name of the sourced file.  Work around that.
  # Note that a bug in some versions of Zsh prevents us from resetting $0
  # in a sourced script, so the use of $argv0.  For more info see:
  #   <http://www.zsh.org/mla/workers/2009/msg01140.html>
  # The apparently useless 'eval' here is needed by at least dash 0.5.2,
  # to prevent it from bailing out with an error like:
  #   "Syntax error: Bad substitution".
  eval 'argv0=${functrace[-1]%:*}' && test -f "$argv0" || {
    echo "Cannot determine the path of running test script." >&2
    echo "Your Zsh (version $ZSH_VERSION) is probably too old." >&2
    exit 99
  }
else
  argv0=$0
  # Ignore command substitution failure, for it might cause problems
  # with "set -e" on some shells.
  am_shell_opts=$(set -o) || :
  case $am_shell_opts in *posix*) set -o posix;; esac
  unset am_shell_opts
fi

# A single whitespace character.
sp=' '
# A tabulation character.
tab='	'
# A newline character.
nl='
'

# As autoconf-generated configure scripts do, ensure that IFS
# is defined initially, so that saving and restoring $IFS works.
IFS=$sp$tab$nl

# The name of the current test (without the '.sh' or '.tap' suffix).
me=${argv0##*/} # Strip all directory components.
case $me in     # Strip test suffix.
   *.tap) me=${me%.tap};;
    *.sh) me=${me%.sh} ;;
 esac

# Source extra package-specific configuration.
. test-defs.sh
# And fail hard if something went wrong.
test $? -eq 0 || exit 99

# We use a trap below for cleanup.  This requires us to go through
# hoops to get the right exit status transported through the signal.
# Turn off errexit here so that we don't trip the bug with OSF1/Tru64
# sh inside this function (FIXME: is this still relevant now that we
# require a POSIX shell?).
_am_exit ()
{
  set +e
  # See comments in the exit trap for the reason we do this.
  test 77 = $1 && am__test_skipped=yes
  # Extra escaping to ensure we do not call our 'exit' alias.
  (\exit $1); \exit $1
}
# Avoid interferences from the environment
am__test_skipped=no
# This alias must actually be placed before any use if 'exit' -- even
# just inside a function definition.  Weird, but real.
alias exit=_am_exit

# In some shells (e.g., Solaris 10 /bin/ksh, or NetBSD 5.1 /bin/sh),
# "unset VAR" returns a non-zero exit status in case the VAR variable
# is already unset.  This doesn't interact well with our usage of
# "set -e" in the testsuite.  This function and the alias below help
# to work around the issue.  But be sure to use them only if actually
# needed.  The repeated unset in the check below cater to the very
# unlikely case where the '_am_v' variable is set in the environment.
if unset _am_v && unset _am_v; then
  : Nothing needs to be done.
else
  _am_unset ()
  {
    for _am_v
    do
      # Extra escaping (here and below) to ensure we do not call our
      # 'unset' alias.
      eval ${_am_v}=dummy && \unset ${_am_v} || return 1
    done
    \unset _am_v
  }
  alias unset=_am_unset
fi

## ------------------------------------ ##
##  General testsuite shell functions.  ##
## ------------------------------------ ##

# Print warnings (e.g., about skipped and failed tests) to this file
# number.  Override by putting, say:
#   AM_TESTS_ENVIRONMENT = stderr_fileno_=9; export stderr_fileno_;
#   AM_TESTS_FD_REDIRECT = 9>&2
# in your Makefile.am.
# This is useful when using automake's parallel tests mode, to print the
# reason for skip/failure to console, rather than to the *.log files.
: ${stderr_fileno_=2}

# Helper functions used by "plain" tests of the Automake testsuite
# (i.e., tests that don't use any test protocol).
# TAP tests will override these functions with their TAP-enhanced
# equivalents later  (see sourcing of 'tap-functions.sh' below).
# These are copied from Gnulib's 'tests/init.sh'.
warn_ () { echo "$@" 1>&$stderr_fileno_; }
fail_ () { warn_ "$me: failed test: $@"; exit 1; }
skip_ () { warn_ "$me: skipped test: $@"; exit 77; }
fatal_ () { warn_ "$me: hard error: $@"; exit 99; }
framework_failure_ () { warn_ "$me: set-up failure: $@"; exit 99; }
# For compatibility with TAP functions.
skip_all_ () { skip_ "$@"; }

if test $am_test_protocol = tap; then
  . tap-functions.sh
fi

## ---------------------------- ##
##  Auxiliary shell functions.  ##
## ---------------------------- ##

# Tell whether we should keep the test directories around, even in
# case of success.  By default, we don't.
am_keeping_testdirs ()
{
  case $keep_testdirs in
     ""|n|no|NO) return 1;;
              *) return 0;;
  esac
}

# seq_ - print a sequence of numbers
# ----------------------------------
# This function simulates GNU seq(1) portably.  Valid usages:
#  - seq LAST
#  - seq FIRST LAST
#  - seq FIRST INCREMENT LAST
seq_ ()
{
  case $# in
    0) fatal_ "seq_: missing argument";;
    1) seq_first=1  seq_incr=1  seq_last=$1;;
    2) seq_first=$1 seq_incr=1  seq_last=$2;;
    3) seq_first=$1 seq_incr=$2 seq_last=$3;;
    *) fatal_ "seq_: too many arguments";;
  esac
  i=$seq_first
  while test $i -le $seq_last; do
    echo $i
    i=$(($i + $seq_incr))
  done
}

# rm_rf_ [FILES OR DIRECTORIES ...]
# ---------------------------------
# Recursively remove the given files or directory, also handling the case
# of non-writable subdirectories.
rm_rf_ ()
{
  test $# -gt 0 || return 0
  $PERL "$am_testaux_srcdir"/deltree.pl "$@"
}

commented_sed_unindent_prog='
  /^$/b                    # Nothing to do for empty lines.
  x                        # Get x<indent> into pattern space.
  /^$/{                    # No prior x<indent>, go prepare it.
    g                      # Copy this 1st non-blank line into pattern space.
    s/^\(['"$tab"' ]*\).*/x\1/   # Prepare x<indent> in pattern space.
  }                        # Now: x<indent> in pattern and <line> in hold.
  G                        # Build x<indent>\n<line> in pattern space, and
  h                        # duplicate it into hold space.
  s/\n.*$//                # Restore x<indent> in pattern space, and
  x                        # exchange with the above duplicate in hold space.
  s/^x\(.*\)\n\1//         # Remove leading <indent> from <line>.
  s/^x.*\n//               # Restore <line> when there is no leading <indent>.
'

# unindent [input files...]
# -------------------------
# Remove the "proper" amount of leading whitespace from the given files,
# and output the result on stdout.  That amount is determined by looking
# at the leading whitespace of the first non-blank line in the input
# files.  If no input file is specified, standard input is implied.
unindent ()
{
  if test x"$sed_unindent_prog" = x; then
    sed_unindent_prog=$(printf '%s\n' "$commented_sed_unindent_prog" \
                          | sed -e "s/  *# .*//")
  fi
  sed "$sed_unindent_prog" ${1+"$@"}
}
sed_unindent_prog="" # Avoid interferences from the environment.

## ---------------------------------------------------------------- ##
##  Create and set up of the temporary directory used by the test.  ##
##  Set up of the exit trap for cleanup of said directory.          ##
## ---------------------------------------------------------------- ##

# Set up the exit trap.
am_exit_trap ()
{
  exit_status=$1
  set +e
  cd "$am_top_builddir"
  if test $am_test_protocol = tap; then
    if test "$planned_" = later && test $exit_status -eq 0; then
      plan_ "now"
    fi
    test $exit_status -eq 0 && test $tap_pass_count_ -eq $tap_count_ \
      || keep_testdirs=yes
  else
    # This is to ensure that a test script does give a SKIP outcome just
    # because a command in it happens to exit with status 77.  This
    # behaviour, while from time to time useful to developers, is not
    # meant to be enabled by default, as it could cause spurious failures
    # in the wild.  Thus it will be enabled only when the variable
    # 'am_explicit_skips' is set to a "true" value.
    case $am_explicit_skips in
      [yY]|[yY]es|1)
        if test $exit_status -eq 77 && test $am__test_skipped != yes; then
          echo "$me: implicit skip turned into failure"
          exit_status=78
        fi;;
    esac
    test $exit_status -eq 0 || keep_testdirs=yes
  fi
  am_keeping_testdirs || rm_rf_ $am_test_subdir
  set +x
  # Spurious escaping to ensure we do not call our "exit" alias.
  \exit $exit_status
}

am_set_exit_traps ()
{
  trap 'am_exit_trap $?' 0
  trap "fatal_ 'caught signal SIGHUP'" 1
  trap "fatal_ 'caught signal SIGINT'" 2
  trap "fatal_ 'caught signal SIGTERM'" 15
  # Various shells seems to just ignore SIGQUIT under some circumstances,
  # even if the signal is not blocked; however, if the signal is trapped,
  # the trap gets correctly executed.  So we also trap SIGQUIT.
  # Here is a list of some shells that have been verified to exhibit the
  # problematic behavior with SIGQUIT:
  #  - zsh 4.3.12 on Debian GNU/Linux
  #  - /bin/ksh and /usr/xpg4/bin/sh on Solaris 10
  #  - Bash 3.2.51 on Solaris 10 and bash 4.1.5 on Debian GNU/Linux
  #  - AT&T ksh on Debian Gnu/Linux (deb package ksh, version 93u-1)
  # OTOH, at least these shells that do *not* exhibit that behaviour:
  #  - modern version of the Almquist Shell (at least 0.5.5.1), on
  #    both Solaris and GNU/Linux
  #  - public domain Korn Shell, version 5.2.14, on Debian GNU/Linux
  trap "fatal_ 'caught signal SIGQUIT'" 3
  # Ignore further SIGPIPE in the trap code.  This is required to avoid
  # a very weird issue with some shells, at least when the execution of
  # the automake testsuite is driven by the 'prove' utility: if prove
  # (or the make process that has spawned it) gets interrupted with
  # Ctrl-C, the shell might go in a loop, continually getting a SIGPIPE,
  # sometimes finally dumping core, other times hanging indefinitely.
  # See also Test::Harness bug [rt.cpan.org #70855], archived at
  # <https://rt.cpan.org/Ticket/Display.html?id=70855>
  trap "trap '' 13; fatal_ 'caught signal SIGPIPE'" 13
}

am_test_setup ()
{
  process_requirements $required
  am_set_exit_traps
  # Create and populate the temporary directory, if required.
  if test x"$am_create_testdir" = x"no"; then
    am_test_subdir=
  else
    am_setup_testdir
  fi
  am_extra_info
  set -x
  pwd
}
