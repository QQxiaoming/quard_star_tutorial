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

# "make uninstall" complains and errors out on failures.

# FIXME: this test only deal with DATA primary; maybe we need sister
# tests for other primaries too?  E.g., SCRIPTS, PROGRAMS, LISP, PYTHON,
# etc...

. test-init.sh

mkdir d
: > d/f
chmod a-w d || skip "cannot make directories unwritable"

# On Solaris 10, if '/bin/rm' is run with the '-f' option, it doesn't
# print any error message when failing to remove a file (due to e.g.,
# "Permission denied").  And it gets weirder.  On OpenIndiana 11, the
# /bin/sh shell (in many respects a decent POSIX shell) seems to somehow
# "eat" the error message from 'rm' in some situation, although the 'rm'
# utility itself correctly prints it when invoked from (say) 'env' or
# 'bash'.  Yikes.
# We'll cater to these incompatibilities by relaxing a test below if
# a faulty shell or 'rm' program is detected.
st=0; $SHELL -c 'rm -f d/f' 2>stderr || st=$?
cat stderr >&2
test $st -gt 0 || skip_ "can delete files from unwritable directories"
if grep 'rm:' stderr; then
  rm_f_is_silent_on_error=no
else
  rm_f_is_silent_on_error=yes
fi

cat >> configure.ac << 'END'
AC_OUTPUT
END

cat > Makefile.am << 'END'
data_DATA = foobar.txt
END

: > foobar.txt

$ACLOCAL
$AUTOMAKE
$AUTOCONF

# Weird name, to make it harder to experience false positives when
# grepping error messages.
inst=__inst-dir__

./configure --prefix="$(pwd)/$inst"

mkdir $inst $inst/share
: > $inst/share/foobar.txt

chmod a-w $inst/share
run_make -M -e FAIL uninstall
if test $rm_f_is_silent_on_error = yes; then
  : "rm -f" is silent on errors, skip the grepping of make output
else
  grep "rm: .*foobar\.txt" output
fi

chmod a-rwx $inst/share
(cd $inst/share) && skip_ "cannot make directories fully unreadable"

run_make -M -e FAIL uninstall

# Some shells, like Solaris 10 /bin/ksh and /usr/xpg4/bin/sh, do not
# report the name of the 'cd' builtin upon a chdir error:
#
#   $ /bin/ksh -c 'cd /none'
#   /bin/ksh: /none: not found
#
# and also print a line number in the error message *if the command
# contains newlines*:
#
#   $ /bin/ksh -c 'cd unreadable'
#   /bin/ksh: unreadable: permission denied
#   $ /bin/ksh -c '\
#   > \
#   > cd unreadable'
#   /bin/ksh[3]: unreadable: permission denied

$EGREP "(cd|sh)(\[[0-9]*[0-9]\])?: .*$inst/share" output

:
