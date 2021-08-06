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
test ${am_test_lib_sourced-no} = yes && return 0
am_test_lib_sourced=yes

# A literal escape character.  Used by test checking colored output.
esc=''

# This might be used in testcases checking distribution-related features.
# Test scripts are free to override this if they need to.
distdir=$me-1.0

## ---------------------- ##
##  Environment cleanup.  ##
## ---------------------- ##

# Unset some make-related variables that may cause $MAKE to act like
# a recursively invoked sub-make.  Any $MAKE invocation in a test is
# conceptually an independent invocation, not part of the main
# 'automake' build.
unset MFLAGS MAKEFLAGS AM_MAKEFLAGS MAKELEVEL
unset __MKLVL__ MAKE_JOBS_FIFO                     # For BSD make.
unset DMAKE_CHILD DMAKE_DEF_PRINTED DMAKE_MAX_JOBS # For Solaris dmake.
# Unset verbosity flag.
unset V
# Also unset variables that might influence "make install".
unset DESTDIR
unset prefix exec_prefix bindir datarootdir datadir docdir dvidir
unset htmldir includedir infodir libdir libexecdir localedir mandir
unset oldincludedir pdfdir psdir sbindir sharedstatedir sysconfdir
# Unset variables that might influence "make distcheck".
unset DISTCHECK_CONFIGURE_FLAGS AM_DISTCHECK_CONFIGURE_FLAGS
# Used by install rules for info files.
unset AM_UPDATE_INFO_DIR
# We don't want to use the $srcdir value exported by the test driver.
unset srcdir
# Also unset variables that control our test driver.  While not
# conceptually independent, they cause some changed semantics we
# need to control (and test for) in some of the tests to ensure
# backward-compatible behavior.
unset TESTS_ENVIRONMENT AM_TESTS_ENVIRONMENT
unset DISABLE_HARD_ERRORS
unset AM_COLOR_TESTS
unset TESTS
unset XFAIL_TESTS
unset TEST_LOGS
unset TEST_SUITE_LOG
unset RECHECK_LOGS
unset VERBOSE
for pfx in TEST_ SH_ TAP_ ''; do
  unset ${pfx}LOG_COMPILER
  unset ${pfx}LOG_COMPILE # Not a typo!
  unset ${pfx}LOG_FLAGS
  unset AM_${pfx}LOG_FLAGS
  unset ${pfx}LOG_DRIVER
  unset ${pfx}LOG_DRIVER_FLAGS
  unset AM_${pfx}LOG_DRIVER_FLAGS
done
unset pfx

# cross_compiling
# ---------------
# Tell whether we are cross-compiling.  This is especially useful to skip
# tests (or portions of them) that requires a native compiler.
cross_compiling ()
{
  # Quoting from the autoconf manual:
  #   ... [$host_alias and $build both] default to the result of running
  #   config.guess, unless you specify either --build or --host.  In
  #   this case, the default becomes the system type you specified.
  #   If you specify both, *and they're different*, configure enters
  #   cross compilation mode (so it doesn't run any tests that require
  #   execution).
  test x"$host_alias" != x && test x"$build_alias" != x"$host_alias"
}

# is_blocked_signal SIGNAL-NUMBER
# --------------------------------
# Return success if the given signal number is blocked in the shell,
# return a non-zero exit status and print a proper diagnostic otherwise.
is_blocked_signal ()
{
  # Use perl, since trying to do this portably in the shell can be
  # very tricky, if not downright impossible.  For reference, see:
  # <https://lists.gnu.org/archive/html/bug-autoconf/2011-09/msg00004.html>
  if $PERL -w -e '
    use strict;
    use warnings FATAL => "all";
    use POSIX;
    my %oldsigaction = ();
    sigaction('"$1"', 0, \%oldsigaction);
    exit ($oldsigaction{"HANDLER"} eq "IGNORE" ? 0 : 77);
  '; then
    return 0
  elif test $? -eq 77; then
    return 1
  else
    fatal_ "couldn't determine whether signal $1 is blocked"
  fi
}

# single_quote STRING
# -------------------
# Single-quote STRING for the shell, also dealing with embedded single
# quotes. Place the result in the '$am_result', that is thus to be
# considered public.
single_quote ()
{
  am_result=$1
  case $am_result in
    *\'*) am_result=$(printf '%s\n' "$*" | sed -e "s/'/'\\\\''/g");;
  esac
  am_result="'$am_result'"
}

# append_single_quoted VARIABLE STRING
# ------------------------------------
append_single_quoted ()
{
  am__var=$1; shift
  single_quote "$1" # Sets 'am_result'.
  eval "${am__var}=\${$am__var:+\"\${$am__var} \"}\$am_result"
  unset am__var am_result
}

# is_valid_varname STRING
# -----------------------
# Tell whether STRING is a valid name for a shell variable.  Return 0
# if yes, return 1 if not.
is_valid_varname ()
{
  # FIXME: is the below truly portable even for LC_COLLATE != "C" ?
  case $1 in
    [0-9]*) return 1;;
    *[!a-zA-Z0-9_]*) return 1;;
  esac
  return 0
}

# run_make [-e STATUS] [-O] [-E] [-M] [--] [VAR=VAL ...] [MAKE-ARGS...]
# ---------------------------------------------------------------------
#
# Run $MAKE with the given command-line, and fail if it doesn't exit with
# STATUS (default: 0).  If STATUS is "FAIL", then any exit status > 0 is
# acceptable.  If STATUS is "IGNORE", any exit value is acceptable.
#
# Other options:
#
#  -O   save the standard output from make on disk, in a regular file
#       named 'stdout'.
#
#  -E   save the standard error from make on disk, in a regular file
#       named 'stderr'.
#
#  -M   save both the standard output and standard error from make on
#       disk, in a regular file named 'output'. This option supersedes
#       both the '-O' and '-E' options.
#
# This function also handle command-line override of variable definition
# in a smart way, using AM_MAKEFLAGS if a non-GNU make implementation
# is in use.
#
run_make ()
{
  am__make_redirect_stdout=no
  am__make_redirect_stderr=no
  am__make_redirect_stdall=no
  am__make_flags=
  am__make_rc_exp=0
  # Follow-up code might want to analyse this, so mark is as
  # publicly accessible (no double undesrscore).
  am_make_rc=0
  # Parse options for this function.
  while test $# -gt 0; do
    case $1 in
      -e) am__make_rc_exp=$2; shift;;
      -O) am__make_redirect_stdout=yes;;
      -E) am__make_redirect_stderr=yes;;
      -M) am__make_redirect_stdall=yes;;
      --) shift; break;;
       *) break;;
    esac
    shift
  done

  # Use append mode here to avoid dropping output.  See automake bug#11413
  if using_gmake; then
    # We can trust GNU make to correctly pass macro definitions given
    # on the command line down to sub-make invocations, and this allow
    # us to have a vary simple implementation: delegate all the work
    # to GNU make.
    :
  else
    # We have to explicitly parse arguments passed to make.  Not 100%
    # safe w.r.t. options like '-I' that can have an argument, but
    # should be good enough for our usages so far.
    for am__x
    do
      case $am__x in
        *=*)
        am__maybe_var=${am__x%%=*}
        am__maybe_val=${am__x#*=}
        am__maybe_def="${am__maybe_var}=${am__maybe_val}"
        # Some variables should be portably overridable from the command
        # line, even when using non-GNU make.
        case $am__maybe_var in
          V|\
          DESTDIR|\
          SHELL|\
          VERBOSE|\
          DISABLE_HARD_ERRORS|\
          DISTCHECK_CONFIGURE_FLAGS)
            ;;
          *)
            if is_valid_varname "$am__maybe_var"; then
              append_single_quoted am__make_flags "$am__maybe_def"
            fi
        esac
        unset am__maybe_var am__maybe_val am__maybe_def
        ;;
      esac
    done
    unset am__x
  fi

  if test x"$am__make_flags" != x; then
     set AM_MAKEFLAGS="$am__make_flags" ${1+"$@"}
     unset am__make_flags
  fi

  # In redirecting make output below, use append mode, to avoid
  # dropping output.  See automake bug#11413 for details.
  # The exit status of 253 is a more-or-less random choice, to
  # help us catch possible errors in redirections and error out
  # accordingly.
  (
    : exec $MAKE ${1+"$@"} # Display traces for future command.
    set +x # We need to remove them now, not to pollute redirected stderr.
    if test $am__make_redirect_stdall = yes; then
      : > output && exec 1>>output 2>&1 || exit 253
    else
      if test $am__make_redirect_stdout = yes; then
        : > stdout && exec 1>>stdout || exit 253
      fi
      if test $am__make_redirect_stderr = yes; then
        : > stderr && exec 2>>stderr || exit 253
      fi
    fi
    exec $MAKE ${1+"$@"}
  ) || am_make_rc=$?

  if test $am_make_rc -eq 253; then
    fatal_ "run_make: problems in redirecting make output"
  fi

  if test $am__make_redirect_stdall = yes; then
    cat output || fatal_ "displaying make output"
  else
    if test $am__make_redirect_stdout = yes; then
      cat stdout || fatal_ "displaying make output"
    fi
    if test $am__make_redirect_stderr = yes; then
      cat stderr >&2 || fatal_ "displaying make output"
    fi
  fi

  case $am__make_rc_exp in
    IGNORE)
      : Ignore exit status
      ;;
    FAIL)
      test $am_make_rc -gt 0 || return 1
      ;;
    *)
     test $am__make_rc_exp -ge 0 && test $am__make_rc_exp -le 255 \
       || fatal_ "invalid expected exit status: '$am__make_rc_exp'"
     test $am_make_rc -eq $am__make_rc_exp || return 1
     ;;
  esac
}

# AUTOMAKE_run [-e STATUS] [-d DESCRIPTION] [--] [AUTOMAKE-ARGS...]
# -----------------------------------------------------------------
# Run automake with AUTOMAKE-ARGS, and fail if it doesn't exit with
# STATUS.  Should be polymorphic for TAP and "plain" tests.  The
# DESCRIPTION, when provided, is used for console reporting, only if
# the TAP protocol is in use in the current test script.
AUTOMAKE_run ()
{
  am__desc=
  am__exp_rc=0
  while test $# -gt 0; do
    case $1 in
      -d) am__desc=$2; shift;;
      -e) am__exp_rc=$2; shift;;
      --) shift; break;;
       # Don't fail on unknown option: assume they (and the rest of the
       # command line) are to be passed verbatim to automake (so stop our
       # own option parsing).
       *) break;;
    esac
    shift
  done
  am__got_rc=0
  $AUTOMAKE ${1+"$@"} >stdout 2>stderr || am__got_rc=$?
  cat stderr >&2
  cat stdout
  if test $am_test_protocol = none; then
    test $am__got_rc -eq $am__exp_rc || exit 1
    return
  fi
  if test -z "$am__desc"; then
    if test $am__got_rc -eq $am__exp_rc; then
      am__desc="automake exited $am__got_rc"
    else
      am__desc="automake exited $am__got_rc, expecting $am__exp_rc"
    fi
  fi
  command_ok_ "$am__desc" test $am__got_rc -eq $am__exp_rc
}

# AUTOMAKE_fails [-d DESCRIPTION] [OPTIONS...]
# --------------------------------------------
# Run automake with OPTIONS, and fail if doesn't exit with status 1.
# Should be polymorphic for TAP and "plain" tests.  The DESCRIPTION,
# when provided, is used for console reporting, only if the TAP
# protocol is in use in the current test script.
AUTOMAKE_fails ()
{
  AUTOMAKE_run -e 1 ${1+"$@"}
}

# extract_configure_help { --OPTION | VARIABLE-NAME } [FILES]
# -----------------------------------------------------------
# Use this to extract from the output of "./configure --help" (or similar)
# the description or help message associated to the given --OPTION or
# VARIABLE-NAME.
extract_configure_help ()
{
  am__opt_re='' am__var_re=''
  case $1 in
    --*'=')   am__opt_re="^  $1";;
    --*'[=]') am__opt_re='^  '$(printf '%s\n' "$1" | sed 's/...$//')'\[=';;
    --*)      am__opt_re="^  $1( .*|$)";;
      *)      am__var_re="^  $1( .*|$)";;
  esac
  shift
  if test x"$am__opt_re" != x; then
    LC_ALL=C awk '
      /'"$am__opt_re"'/        { print; do_print = 1; next; }
      /^$/                     { do_print = 0; next }
      /^  --/                  { do_print = 0; next }
      (do_print == 1)          { print }
    ' ${1+"$@"}
  else
    LC_ALL=C awk '
      /'"$am__var_re"'/        { print; do_print = 1; next; }
      /^$/                     { do_print = 0; next }
      /^  [A-Z][A-Z0-9_]* /    { do_print = 0; next }
      /^  [A-Z][A-Z0-9_]*$/    { do_print = 0; next }
      (do_print == 1)          { print }
    ' ${1+"$@"}
  fi
}

# grep_configure_help { --OPTION | VARIABLE-NAME } REGEXP
# -------------------------------------------------------
# Grep the section of "./configure --help" output associated with either
# --OPTION or VARIABLE-NAME for the given *extended* regular expression.
grep_configure_help ()
{
  ./configure --help > am--all-help \
    || { cat am--all-help; exit 1; }
  cat am--all-help
  extract_configure_help "$1" am--all-help > am--our-help \
    || { cat am--our-help; exit 1; }
  cat am--our-help
  $EGREP "$2" am--our-help || exit 1
}

# using_gmake
# -----------
# Return success if $MAKE is GNU make, return failure otherwise.
# Caches the result for speed reasons.
using_gmake ()
{
  case $am__using_gmake in
    yes)
      return 0;;
    no)
      return 1;;
    '')
      # Use --version AND -v, because SGI Make doesn't fail on --version.
      # Also grep for GNU because newer versions of FreeBSD make do
      # not complain about --version (they seem to silently ignore it).
      if $MAKE --version -v | grep GNU; then
        am__using_gmake=yes
        return 0
      else
        am__using_gmake=no
        return 1
      fi;;
    *)
      fatal_ "invalid value for \$am__using_gmake: '$am__using_gmake'";;
  esac
}
am__using_gmake="" # Avoid interferences from the environment.

# make_can_chain_suffix_rules
# ---------------------------
# Return 0 if $MAKE is a make implementation that can chain suffix rules
# automatically, return 1 otherwise.  Caches the result for speed reasons.
make_can_chain_suffix_rules ()
{
  if test -z "$am__can_chain_suffix_rules"; then
    if using_gmake; then
      am__can_chain_suffix_rules=yes
      return 0
    else
      mkdir am__chain.dir$$
      cd am__chain.dir$$
      unindent > Makefile << 'END'
        .SUFFIXES: .u .v .w
        .u.v: ; cp $< $@
        .v.w: ; cp $< $@
END
      echo make can chain suffix rules > foo.u
      if $MAKE foo.w && diff foo.u foo.w; then
        am__can_chain_suffix_rules=yes
      else
        am__can_chain_suffix_rules=no
      fi
      cd ..
      rm -rf am__chain.dir$$
    fi
  fi
  case $am__can_chain_suffix_rules in
    yes) return 0;;
     no) return 1;;
      *) fatal_ "make_can_chain_suffix_rules: internal error";;
  esac
}
am__can_chain_suffix_rules="" # Avoid interferences from the environment.

# useless_vpath_rebuild
# ---------------------
# Tell whether $MAKE suffers of the bug triggering automake bug#7884.
# For example, this happens with FreeBSD make, since in a VPATH build
# it tends to rebuilt files for which there is an explicit or even just
# a suffix rule, even if said files are already available in the VPATH
# directory.
useless_vpath_rebuild ()
{
  if test -z "$am__useless_vpath_rebuild"; then
    if using_gmake; then
      am__useless_vpath_rebuild=no
      return 1
    fi
    mkdir am__vpath.dir$$
    cd am__vpath.dir$$
    touch foo.a foo.b bar baz
    mkdir build
    cd build
    unindent > Makefile << 'END'
        .SUFFIXES: .a .b
        VPATH = ..
        all: foo.b baz
        .PHONY: all
        .a.b: ; cp $< $@
        baz: bar ; cp ../baz bar
END
    if run_make all && test ! -e foo.b && test ! -e bar; then
      am__useless_vpath_rebuild=no
    else
      am__useless_vpath_rebuild=yes
    fi
    cd ../..
    rm -rf am__vpath.dir$$
  fi
  case $am__useless_vpath_rebuild in
    yes) return 0;;
     no) return 1;;
     "") ;;
      *) fatal_ "useless_vpath_rebuild: internal error";;
  esac
}
am__useless_vpath_rebuild=""

yl_distcheck () { useless_vpath_rebuild || run_make distcheck ${1+"$@"}; }

null_install ()
{
  for am__v in nulldirs destdir instdir; do
    if ! eval 'test -n "$'$am__v'"'; then
      fatal_ "null_install() invoked with \$$am__v unset"
    fi
  done
  unset am__v
  case $#,$1 in
    0,)
      am__inst='install';;
    1,-t|1,--texi)
      am__inst='install install-html install-dvi install-ps install-pdf';;
    *)
      fatal_ "null_install(): invalid usage";;
  esac
  run_make $nulldirs $am__inst
  test ! -e "$instdir"
  run_make $nulldirs $am__inst DESTDIR="$destdir"
  test ! -e "$instdir"
  test ! -e "$destdir"
  run_make -M $nulldirs uninstall
  # Creative quoting below to please maintainer-check.
  grep 'rm'' ' output && exit 1
  run_make -M $nulldirs uninstall DESTDIR="$destdir"
  # Creative quoting below to please maintainer-check.
  grep 'rm'' ' output && exit 1
  : # For 'set -e'.
}

# count_test_results total=N pass=N fail=N xpass=N xfail=N skip=N error=N
# -----------------------------------------------------------------------
# Check that a testsuite run driven by the parallel-tests harness has
# had the specified numbers of test results (specified by kind).
# This function assumes that the output of "make check" or "make recheck"
# has been saved in the 'stdout' file in the current directory, and its
# log in the 'test-suite.log' file.
count_test_results ()
{
  # Use a subshell so that we won't pollute the script namespace.
  (
    # TODO: Do proper checks on the arguments?
    total=ERR pass=ERR fail=ERR xpass=ERR xfail=ERR skip=ERR error=ERR
    eval "$@"
    # For debugging.
    $EGREP -i '(total|x?pass|x?fail|skip|error)' stdout || :
    rc=0
    # Avoid spurious failures with shells with "overly sensible"
    # errexit shell flag, such as e.g., Solaris /bin/sh.
    set +e
    test $(grep -c '^PASS:'  stdout) -eq $pass  || rc=1
    test $(grep -c '^XFAIL:' stdout) -eq $xfail || rc=1
    test $(grep -c '^SKIP:'  stdout) -eq $skip  || rc=1
    test $(grep -c '^FAIL:'  stdout) -eq $fail  || rc=1
    test $(grep -c '^XPASS:' stdout) -eq $xpass || rc=1
    test $(grep -c '^ERROR:' stdout) -eq $error || rc=1
    grep "^# TOTAL:  *$total$" stdout || rc=1
    grep "^# PASS:  *$pass$"   stdout || rc=1
    grep "^# XFAIL:  *$xfail$" stdout || rc=1
    grep "^# SKIP:  *$skip$"   stdout || rc=1
    grep "^# FAIL:  *$fail$"   stdout || rc=1
    grep "^# XPASS:  *$xpass$" stdout || rc=1
    grep "^# ERROR:  *$error$" stdout || rc=1
    test $rc -eq 0
  )
}

# get_shell_script SCRIPT-NAME
# -----------------------------
# Fetch an Automake-provided shell script from the 'lib/' directory into
# the current directory, and, if the '$am_test_prefer_config_shell'
# variable is set to "yes", modify its shebang line to use $SHELL instead
# of /bin/sh.
get_shell_script ()
{
  am_source=$1 am_target=${2-$1}
  test ! -f "$am_target" || rm -f "$am_target" || return 99
  if test x"$am_test_prefer_config_shell" = x"yes"; then
    sed "1s|#!.*|#! $SHELL|" "$am_scriptdir/$am_source" > "$am_target" \
     && chmod a+x "$am_target" \
     || return 99
  else
    cp -f "$am_scriptdir/$am_source" "$am_target" || return 99
  fi
  sed 10q "$am_target" # For debugging.
  unset am_target am_source
}

# fetch_tap_driver
# ----------------
# Fetch the Automake-provided TAP driver from the 'lib/' directory into
# the current directory, and edit its shebang line so that it will be
# run with the proper shell.
fetch_tap_driver ()
{
  AM_TAP_AWK=$AWK; export AM_TAP_AWK
  get_shell_script tap-driver.sh tap-driver
}


# require_xsi SHELL
# -----------------
# Skip the test if the given shell fails to support common XSI constructs.
require_xsi ()
{
  test $# -eq 1 || fatal_ "require_xsi needs exactly one argument"
  echo "$me: trying some XSI constructs with $1"
  $1 -c "$xsi_shell_code" || skip_all_ "$1 lacks XSI features"
}
# Shell code supposed to work only with XSI shells.  Keep this in sync
# with libtool.m4:_LT_CHECK_SHELL_FEATURES.
xsi_shell_code='
  _lt_dummy="a/b/c"
  test "${_lt_dummy##*/},${_lt_dummy%/*},${_lt_dummy#??}"${_lt_dummy%"$_lt_dummy"}, \
      = c,a/b,b/c, \
    && eval '\''test $(( 1 + 1 )) -eq 2 \
    && test "${#_lt_dummy}" -eq 5'\'

# $PYTHON and support for PEP-3147.  Needed to check our python-related
# install rules.
python_has_pep3147 ()
{
  if test -z "$am_pep3147_tag"; then
    am_pep3147_tag=$($PYTHON -c 'import imp; print(imp.get_tag())') \
      || am_pep3147_tag=none
  fi
  test $am_pep3147_tag != none
}
am_pep3147_tag=

# pyc_location [-p] [FILE]
# ------------------------
# Determine what the actual location of the given '.pyc' or '.pyo'
# byte-compiled file should be, taking into account PEP-3147.  Save
# the location in the '$am_pyc_file' variable.  If the '-p' option
# is given, print the location on the standard output as well.
pyc_location ()
{
  case $#,$1 in
    2,-p) am_pyc_print=yes; shift;;
     1,*) am_pyc_print=no;;
       *) fatal_ "pyc_location: invalid usage";;
  esac
  if python_has_pep3147; then
    case $1 in
      */*) am_pyc_dir=${1%/*} am_pyc_base=${1##*/};;
        *) am_pyc_dir=. am_pyc_base=$1;;
    esac
    am_pyc_ext=${am_pyc_base##*.}
    am_pyc_base=${am_pyc_base%.py?}
    am_pyc_file=$am_pyc_dir/__pycache__/$am_pyc_base.$am_pep3147_tag.$am_pyc_ext
  else
    am_pyc_file=$1
  fi
  test $am_pyc_print = no || printf '%s\n' "$am_pyc_file"
}

# py_installed [--not] FILE
# --------------------------
# Check that the given python FILE has been installed (resp. *not*
# installed, if the '--not' option is specified).  If FILE is a
# byte-compiled '.pyc' file, the new installation layout specified
# by PEP-3147 will be taken into account.
py_installed ()
{
  case $#,$1 in
        1,*) am_test_py_file='test -f';;
    2,--not) am_test_py_file='test ! -e'; shift;;
          *) fatal_ "pyc_installed: invalid usage";;
  esac
  case $1 in
    *.py[co]) pyc_location "$1"; am_target_py_file=$am_pyc_file;;
           *) am_target_py_file=$1;;
  esac
  $am_test_py_file "$am_target_py_file"
}

# Usage: require_compiler_ {cc|c++|fortran|fortran77}
require_compiler_ ()
{
  case $# in
    0) fatal_ "require_compiler_: missing argument";;
    1) ;;
    *) fatal_ "require_compiler_: too many arguments";;
  esac
  case $1 in
    cc)
      am__comp_lang="C"
      am__comp_var=CC
      am__comp_flag_vars='CFLAGS CPPFLAGS'
      ;;
    c++)
      am__comp_lang="C++"
      am__comp_var=CXX
      am__comp_flag_vars='CXXFLAGS CPPFLAGS'
      ;;
    fortran)
      am__comp_lang="Fortran"
      am__comp_var=FC
      am__comp_flag_vars='FCFLAGS'
      ;;
    fortran77)
      am__comp_lang="Fortran 77"
      am__comp_var=F77
      am__comp_flag_vars='FFLAGS'
      ;;
  esac
  shift
  eval "am__comp_prog=\${$am__comp_var}" \
    || fatal_ "expanding \${$am__comp_var} in require_compiler_"
  case $am__comp_prog in
    "")
      fatal_ "botched configuration: \$$am__comp_var is empty";;
    false)
      skip_all_ "no $am__comp_lang compiler available";;
    autodetect|autodetected)
      # Let the ./configure commands in the test script try to determine
      # these automatically.
      unset $am__comp_var $am__comp_flag_vars;;
    *)
      # Pre-set these for the ./configure commands in the test script.
      export $am__comp_var $am__comp_flag_vars;;
  esac
  # Delete private variables.
  unset am__comp_lang am__comp_prog am__comp_var am__comp_flag_vars
}

## ----------------------------------------------------------- ##
##  Checks for required tools, and additional setups (if any)  ##
##  required by them.                                          ##
## ----------------------------------------------------------- ##

require_tool ()
{
  am_tool=$1
  case $1 in
    cc|c++|fortran|fortran77)
      require_compiler_ $1;;
    -c-o)
      if test x"$AM_TESTSUITE_SIMULATING_NO_CC_C_O" = x"yes"; then
        skip_all_ "need a C compiler that grasps -c and -o together"
      fi
      ;;
    xsi-lib-shell)
      if test x"$am_test_prefer_config_shell" = x"yes"; then
        require_xsi "$SHELL"
      else
        require_xsi "/bin/sh"
      fi
      ;;
    bzip2)
      # Do not use --version, older versions bzip2 still tries to compress
      # stdin.
      echo "$me: running bzip2 --help"
      bzip2 --help \
        || skip_all_ "required program 'bzip2' not available"
      ;;
    cl)
      CC=cl
      # Don't export CFLAGS, as that could have been initialized to only
      # work with the C compiler detected at configure time.  If the user
      # wants CFLAGS to also influence 'cl', he can still export CFLAGS
      # in the environment "by hand" before calling the testsuite.
      export CC CPPFLAGS
      echo "$me: running $CC -?"
      # The IRAF package (http://iraf.noao.edu/) contains a 'cl' program
      # which is interactive, and which could cause the testsuite to hang
      # if its standard input is not redirected.  See automake bug#14707.
      $CC -? </dev/null \
        || skip_all_ "Microsoft C compiler '$CC' not available"
      ;;
    icl)
      CC=icl
      # Don't export CFLAGS, as that could have been initialized to only
      # work with the C compiler detected at configure time.  If the user
      # wants CFLAGS to also influence 'icl', he can still export CFLAGS
      # in the environment "by hand" before calling the testsuite.
      export CC CPPFLAGS
      echo "$me: running $CC -?"
      $CC -? >/dev/null \
        || skip_all_ "Intel C compiler '$CC' not available"
      ;;
    etags)
      # Exuberant Ctags will create a TAGS file even
      # when asked for --help or --version.  (Emacs's etags
      # does not have such problem.)  Use -o /dev/null
      # to make sure we do not pollute the build directory.
      echo "$me: running etags --version -o /dev/null"
      etags --version -o /dev/null \
        || skip_all_ "required program 'etags' not available"
      ;;
    GNUmake)
      for am_make in "$MAKE" gmake gnumake :; do
        MAKE=$am_make
        am__using_gmake= # Invalidate cache used by 'using_gmake()'.
        test "$MAKE" = : && break
        echo "$me: determine whether $MAKE is GNU make"
        using_gmake && break
        : For shells with busted 'set -e'.
      done
      test "$MAKE" = : && skip_all_ "this test requires GNU make"
      export MAKE
      unset am_make
      ;;
    gcj)
      GCJ=$GNU_GCJ GCJFLAGS=$GNU_GCJFLAGS; export GCJ GCJFLAGS
      test "$GCJ" = false && skip_all_ "GNU Java compiler unavailable"
      : For shells with busted 'set -e'.
      ;;
    gcc)
      CC=$GNU_CC CFLAGS=$GNU_CFLAGS; export CC CFLAGS CPPFLAGS
      test "$CC" = false && skip_all_ "GNU C compiler unavailable"
      : For shells with busted 'set -e'.
      ;;
    g++)
      CXX=$GNU_CXX CXXFLAGS=$GNU_CXXFLAGS; export CXX CXXFLAGS CPPFLAGS
      test "$CXX" = false && skip_all_ "GNU C++ compiler unavailable"
      : For shells with busted 'set -e'.
      ;;
    gfortran)
      FC=$GNU_FC FCFLAGS=$GNU_FCFLAGS; export FC FCFLAGS
      test "$FC" = false && skip_all_ "GNU Fortran compiler unavailable"
      case " $required " in
        *\ g77\ *) ;;
        *) F77=$FC FFLAGS=$FCFLAGS; export F77 FFLAGS;;
      esac
      ;;
    g77)
      F77=$GNU_F77 FFLAGS=$GNU_FFLAGS; export F77 FFLAGS
      test "$F77" = false && skip_all_ "GNU Fortran 77 compiler unavailable"
      case " $required " in
        *\ gfortran\ *) ;;
        *) FC=$F77 FCFLAGS=$FFLAGS; export FC FCFLAGS;;
      esac
      ;;
    grep-nonprint)
      # Check that grep can parse nonprinting characters correctly.
      # BSD 'grep' works from a pipe, but not a seekable file.
      # GNU or BSD 'grep -a' works on files, but is not portable.
      case $(echo "$esc" | grep .)$(echo "$esc" | grep "$esc") in
        "$esc$esc") ;;
        *) skip_ "grep can't handle nonprinting characters correctly";;
      esac
      ;;
    javac)
      # The Java compiler from JDK 1.5 (and presumably earlier versions)
      # cannot handle the '-version' option by itself: it bails out
      # telling that source files are missing.  Adding also the '-help'
      # option seems to solve the problem.
      echo "$me: running javac -version -help"
      javac -version -help || skip_all_ "Sun Java compiler not available"
      ;;
    java)
      # See the comments above about 'javac' for why we use also '-help'.
      echo "$me: running java -version -help"
      java -version -help || skip_all_ "Sun Java interpreter not found"
      ;;
    lib)
      AR=lib; export AR
      # Attempting to create an empty archive will actually not
      # create the archive, but lib will output its version.
      echo "$me: running $AR -out:defstest.lib"
      $AR -out:defstest.lib \
        || skip_all_ "Microsoft 'lib' utility not available"
      ;;
    makedepend)
      echo "$me: running makedepend -f-"
      makedepend -f- \
        || skip_all_ "required program 'makedepend' not available"
      ;;
    mingw)
      uname_s=$(uname -s || echo UNKNOWN)
      echo "$me: system name: $uname_s"
      case $uname_s in
        MINGW*) ;;
        *) skip_all_ "this test requires MSYS in MinGW mode" ;;
      esac
      unset uname_s
      ;;
    non-root)
      # Skip this test case if the user is root.
      # We try to append to a read-only file to detect this.
      priv_check_temp=am--priv-check.$$
      touch $priv_check_temp && chmod a-w $priv_check_temp \
        || framework_failure_ "creating unwritable file $priv_check_temp"
      # Not a useless use of subshell: lesser shells might bail
      # out if a builtin fails.
      overwrite_status=0
      (echo foo >> $priv_check_temp) || overwrite_status=$?
      rm -f $priv_check_temp
      if test $overwrite_status -eq 0; then
        skip_all_ "cannot drop file write permissions"
      fi
      unset priv_check_temp overwrite_status
      ;;
    # Extra quoting required to avoid maintainer-check spurious failures.
    'perl-threads')
      if test "$WANT_NO_THREADS" = "yes"; then
        skip_all_ "Devel::Cover cannot cope with threads"
      fi
      ;;
    native)
      # Don't use "&&" here, to avoid a bug of 'set -e' present in
      # some (even relatively recent) versions of the BSD shell.
      # We add the dummy "else" branch for extra safety.
      ! cross_compiling || skip_all_ "doesn't work in cross-compile mode"
      ;;
    python)
      PYTHON=${PYTHON-python}
      # Older python versions don't support --version, they have -V.
      echo "$me: running $PYTHON -V"
      $PYTHON -V || skip_all_ "python interpreter not available"
      ;;
    ro-dir)
      # Skip this test case if read-only directories aren't supported
      # (e.g., under DOS.)
      ro_dir_temp=ro_dir.$$
      mkdir $ro_dir_temp && chmod a-w $ro_dir_temp \
        || framework_failure_ "creating unwritable directory $ro_dir_temp"
      # Not a useless use of subshell: lesser shells might bail
      # out if a builtin fails.
      create_status=0
      (: > $ro_dir_temp/probe) || create_status=$?
      rm -rf $ro_dir_temp
      if test $create_status -eq 0; then
        skip_all_ "cannot drop directory write permissions"
      fi
      unset ro_dir_temp create_status
      ;;
    runtest)
      # DejaGnu's runtest program. We rely on being able to specify
      # the program on the runtest command-line. This requires
      # DejaGnu 1.4.3 or later.
      echo "$me: running runtest SOMEPROGRAM=someprogram --version"
      runtest SOMEPROGRAM=someprogram --version \
        || skip_all_ "DejaGnu is not available"
      ;;
    tex)
      # No all versions of Tex support '--version', so we use
      # a configure check.
      if test -z "$TEX"; then
        skip_all_ "TeX is required, but it wasn't found by configure"
      fi
      ;;
    lex)
      test x"$LEX" = x"false" && skip_all_ "lex not found or disabled"
      export LEX
      ;;
    yacc)
      test x"$YACC" = x"false" && skip_all_ "yacc not found or disabled"
      export YACC
      ;;
    flex)
      LEX=flex; export LEX
      echo "$me: running flex --version"
      flex --version || skip_all_ "required program 'flex' not available"
      ;;
    bison)
      YACC='bison -y'; export YACC
      echo "$me: running bison --version"
      bison --version || skip_all_ "required program 'bison' not available"
      ;;
    valac)
      echo "$me: running valac --version"
      if ! valac --version; then
        skip_all_ "required program 'valac' not available"
      elif cross_compiling; then
        skip_all_ "cross-compiling valac-generated C files is brittle"
      fi
      # TODO: We also know we need GNU make, the C compiler, and pkg-config
      # here, but there is no easy way to express this with the current
      # code organization.  We should improve the situation, sooner or
      # later.  At which point the tests requiring 'valac' can drop the
      # explicit requirements for those tools.
      ;;
    *)
      # Generic case: the tool must support --version.
      echo "$me: running $1 --version"
      # It is not likely but possible that the required tool is a special
      # builtin, in which case the shell is allowed to exit after an error.
      # So we need the subshell here.  Also, some tools, like Sun cscope,
      # can be interactive without redirection.
      ($1 --version) </dev/null \
        || skip_all_ "required program '$1' not available"
      ;;
  esac
}

process_requirements ()
{
  # Look for (and maybe set up) required tools and/or system features;
  # skip the current test if they are not found.
  for am_tool in $*; do
    require_tool $am_tool
  done
  # We might need extra m4 macros, e.g., for Libtool or Gettext.
  for am_tool in gettext libtool pkg-config; do
    case " $required " in
      # The lack of whitespace after $am_tool is intended.
      *" $am_tool"*) . ./t/$am_tool-macros.dir/get.sh;;
    esac
  done
  unset am_tool
}

## ---------------------------------------------------------------- ##
##  Create and set up of the temporary directory used by the test.  ##
## ---------------------------------------------------------------- ##

am_setup_testdir ()
{
  # The subdirectory where the current test script will run and write its
  # temporary/data files.  This will be created shortly, and will be removed
  # by the cleanup trap below if the test passes.  If the test doesn't pass,
  # this directory will be kept, to facilitate debugging.
  am_test_subdir=${argv0#$am_rel_srcdir/}
  case $am_test_subdir in
    */*) am_test_subdir=${am_test_subdir%/*}/$me.dir;;
      *) am_test_subdir=$me.dir;;
  esac
  test ! -e $am_test_subdir || rm_rf_ $am_test_subdir \
    || framework_failure_ "removing old test subdirectory"
  $MKDIR_P $am_test_subdir \
    || framework_failure_ "creating test subdirectory"
  cd $am_test_subdir \
    || framework_failure_ "cannot chdir into test subdirectory"
  if test x"$am_create_testdir" != x"empty"; then
    cp "$am_scriptdir"/install-sh "$am_scriptdir"/missing \
       "$am_scriptdir"/compile "$am_scriptdir"/depcomp . \
      || framework_failure_ "fetching common files from $am_scriptdir"
    # Build appropriate environment in test directory.  E.g., create
    # configure.ac, touch all necessary files, etc.  Don't use AC_OUTPUT,
    # but AC_CONFIG_FILES so that appending still produces a valid
    # configure.ac.  But then, tests running config.status really need
    # to append AC_OUTPUT.
    {
      echo "AC_INIT([$me], [1.0])"
      if test x"$am_serial_tests" = x"yes"; then
        echo "AM_INIT_AUTOMAKE([serial-tests])"
      else
        echo "AM_INIT_AUTOMAKE"
      fi
      echo "AC_CONFIG_FILES([Makefile])"
    } >configure.ac || framework_failure_ "creating configure.ac skeleton"
  fi
}

am_extra_info ()
{
  echo "Running from installcheck: $am_running_installcheck"
  echo "Test Protocol: $am_test_protocol"
  echo "PATH = $PATH"
}
