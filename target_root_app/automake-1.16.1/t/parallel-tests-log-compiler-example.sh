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

# Test the example of usage of generic and extension-specific
# LOG_COMPILER and LOG_FLAGS given in the manual.

required=python
. test-init.sh

cat >> configure.ac <<END
AC_SUBST([PERL], ['$PERL'])
AM_PATH_PYTHON
AC_OUTPUT
END

cat > Makefile.am << 'END'
TESTS = foo.pl bar.py baz
TEST_EXTENSIONS = .pl .py
PL_LOG_COMPILER = $(PERL)
AM_PL_LOG_FLAGS = -w
PY_LOG_COMPILER = $(PYTHON)
AM_PY_LOG_FLAGS = -v
LOG_COMPILER = ./wrapper-script
AM_LOG_FLAGS = -d
END

echo 'my $a =+ 2; exit (0);' > foo.pl
echo 'import sys; sys.exit(0);' > bar.py
: > baz

cat > wrapper-script <<'END'
#!/bin/sh
echo "wrapper args: $*"
END
chmod a+x wrapper-script

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure

st=0
$MAKE check || st=$?
cat foo.log
cat bar.log
cat baz.log
test $st -eq 0 || exit $st

# Check that the wrappers have been run with the expected flags.
grep '[rR]eversed.*+=.*operator.*foo\.pl' foo.log
grep '^# *[cC]lear.*sys\.argv' bar.log
grep '^wrapper args:.* -d .*baz' baz.log

:
