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

# Custom test drivers: check that the test name passed to the test
# driver has any VPATH prefix stripped.

. test-init.sh

ocwd=$(pwd) || fatal_ "cannot get current working directory"

mkdir src build
mv install-sh missing configure.ac src
rm -f depcomp

cd src

cat >> configure.ac << 'END'
AC_OUTPUT
END

cat > Makefile.am << 'END'
# The directories of the 'bar.test' and 'baz.test' tests are deliberately
# called as the source directory, to verify that the VPATH-stripping code
# doesn't get too easily confused.
# The $(empty) are for eliciting VPATH rewrites on make implementations
# that support it (e.g., Solaris make), to improve coverage.
empty =
TESTS = $(empty) foo.test src/bar.test ./src/baz.test $(empty)
$(TESTS):
TEST_LOG_DRIVER = $(srcdir)/checkstrip-driver
EXTRA_DIST = checkstrip-driver
END

cat > checkstrip-driver <<'END'
#! /bin/sh
set -e
while test $# -gt 0; do
  case $1 in
    --log-file) log_file=$2; shift;;
    --trs-file) trs_file=$2; shift;;
    --test-name) test_name=$2; shift;;
    --expect-failure|--color-tests|--enable-hard-errors) shift;;
    --) shift; break;;
     *) echo "$0: invalid option/argument: '$1'" >&2; exit 2;;
  esac
  shift
done
echo "test name: $test_name" # For debugging.
case $test_name in
  foo.test|./foo.test|src/ba[rz].test|./src/ba[rz].test);;
  *) exit 1;;
esac
echo dummy > "$log_file"
echo dummy > "$trs_file"
END
chmod a+x checkstrip-driver

$ACLOCAL
$AUTOCONF
$AUTOMAKE

cd ..

mkdir build1
cd build1
../src/configure
# "$MAKE -n" is for debugging, should highlight any VPATH rewrite.
$MAKE -n check || :
$MAKE check
cd ..

mkdir build2
cd build2
"$ocwd"/src/configure
# "$MAKE -n" is for debugging, should highlight any VPATH rewrite.
$MAKE -n check || :
$MAKE check
cd ..

cd src
./configure
$MAKE distcheck

:
