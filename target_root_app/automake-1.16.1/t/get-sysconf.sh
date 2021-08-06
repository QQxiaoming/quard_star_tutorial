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

# Dummy test case, aimed at reporting useful system information in the
# final 'test-suite.log'.  This way, if a user experiences a failure in
# the Automake test suite and then only sends us the 'test-suite.log',
# we won't have to ask him for more information to start analyzing the
# failure (that had happened too many times in the past!).

. test-init.sh

st=0
if test -d "$am_top_srcdir"/.git; then
  # We are running from a git checkout.
  (cd "$am_top_srcdir" && git log -1) || st=1
elif test -f "$am_top_srcdir"/ChangeLog; then
  # We are probably running from a distribution tarball.
  awk '
    BEGIN { first = 1 }
    (first == 1) { print; first = 0; next; }
    /^[^\t]/ { exit(0); }
    { print }
  ' "$am_top_srcdir"/ChangeLog || st=1
else
  # Some non-common but possibly valid setup (see for example the Homebrew
  # problem reported in automake bug#10866); so just give an harmless
  # warning instead of failing.
  warn_ "no .git directory nor ChangeLog file found, some info won't" \
        "be available"
fi
$PERL -V || st=1
# It's OK for the user not to have the TAP::Parser module; this can
# happen with older perl installation, or on MinGW/MSYS.
$PERL -e 'use TAP::Parser; print $TAP::Parser::VERSION, "\n"' || :

# It's OK if the selected Lex and Yacc programs don't know how to print
# the version number or the help screen; those are usually available only
# for Flex and Bison.
$LEX --version || :
$LEX --help || :
$YACC --version || :
$YACC --help || :

cat "$am_top_builddir/config.log" || st=1
cat "$am_top_builddir/bin/aclocal-$APIVERSION" || st=1
cat "$am_top_builddir/bin/automake-$APIVERSION" || st=1

if test $st -eq 0; then
  # This test SKIPs, so that all the information it has gathered and
  # printed will get unconditionally copied into the 'test-suite.log'
  # file.
  exit 77
fi

# Some unexpected error occurred; this must be reported as an hard
# error by the testsuite driver.
exit 99
