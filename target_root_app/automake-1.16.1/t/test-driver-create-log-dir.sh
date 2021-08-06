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

# Custom test drivers: if a log file has a directory component (as in
# e.g., 'sub/foo.log'), the Automake test harness must ensure that
# directory exists before calling any custom test driver.

. test-init.sh

cat >> configure.ac << 'END'
AC_OUTPUT
END

cat > Makefile.am << 'END'
# The extra '.' components below are meant.
TESTS = sub/foo sub/subsub/bar.test ././sub1/./baz
$(TESTS):
LOG_DRIVER = $(srcdir)/checkdir-driver
TEST_LOG_DRIVER = $(LOG_DRIVER)
EXTRA_DIST = checkdir-driver

check-local: $(TEST_SUITE_LOG)
	test -d sub
	test -d sub1
	test -d sub/subsub
	test -f sub/foo.log
	test -f sub/subsub/bar.log
	test -f sub1/baz.log
	test -f sub/foo.trs
	test -f sub/subsub/bar.trs
	test -f sub1/baz.trs
END

echo "#!$AM_TEST_RUNNER_SHELL" > checkdir-driver
cat >> checkdir-driver <<'END'
set -e; set -u
while test $# -gt 0; do
  case $1 in
    --log-file) log_file=$2; shift;;
    --trs-file) trs_file=$2; shift;;
    --test-name|--expect-failure|--color-tests|--enable-hard-errors) shift;;
    --) shift; break;;
     *) echo "$0: invalid option/argument: '$1'" >&2; exit 2;;
  esac
  shift
done
echo "log: $log_file" # For debugging.
echo "trs: $trs_file" # Ditto.
case $log_file in */*);; *) exit 1;; esac
dir_log=${log_file%/*}
dir_trs=${trs_file%/*}
echo "dir_log: $dir_log" # For debugging.
echo "dir_trs: $dir_trs" # Likewise.
test x"$dir_trs" = x"$dir_log" || exit 1
test -d "$dir_log" || exit 1
echo dummy1 > "$log_file"
echo dummy2 > "$trs_file"
END
chmod a+x checkdir-driver

$ACLOCAL
$AUTOCONF
$AUTOMAKE

./configure
$MAKE check
$MAKE distcheck

:
