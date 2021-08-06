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

# Custom test drivers: per-extension test drivers.

. test-init.sh

cat >> configure.ac << 'END'
AC_OUTPUT
END

cat > Makefile.am << 'END'
# Automake shouldn't match the '/test' in 'sub/test' as '.test' suffix.
TESTS = 1.chk 2.test 3 4.c.chk 5.suf sub/test

TEST_EXTENSIONS = .chk .test

CHK_LOG_DRIVER = ./chk-wrapper
TEST_LOG_DRIVER = $(SHELL) $(srcdir)/test-wrapper
LOG_DRIVER = noext-wrapper

AM_CHK_LOG_DRIVER_FLAGS = --am-chk
CHK_LOG_DRIVER_FLAGS = --chk
AM_TEST_LOG_DRIVER_FLAGS = -am-test
TEST_LOG_DRIVER_FLAGS = -test
AM_LOG_DRIVER_FLAGS = am
LOG_DRIVER_FLAGS = _
END

mkdir sub bin
PATH=$(pwd)/bin$PATH_SEPARATOR$PATH; export PATH

cat > wrapper.skel <<'END'
#! /bin/sh
set -e

me=`echo "$0" | sed 's,^.*/,,'`
if test -z "$me"; then
  echo "$0: cannot determine program name" >&2
  exit 99
fi

am_log_wflags='@am_log_wflags@'
log_wflags='@log_wflags@'

test_name=INVALID
log_file=BAD.log
trs_file=BAD.trs
extra_opts=
while test $# -gt 0; do
  case $1 in
    --test-name) test_name=$2; shift;;
    --log-file) log_file=$2; shift;;
    --trs-file) trs_file=$2; shift;;
    # Ignored.
    --expect-failure) shift;;
    --color-tests) shift;;
    --enable-hard-errors) shift;;
    # Remembered in the same order they're passed in.
    $am_log_wflags|$log_wflags) extra_opts="$extra_opts $1";;
    # Explicitly terminate option list.
    --) shift; break;;
    # Shouldn't happen
    *) echo "$0: invalid option/argument: '$1'" >&2; exit 2;;
  esac
  shift
done

echo "$me" "$test_name" $extra_opts > "$log_file"
: > "$trs_file"

exec "$@"
exit 127
END

sed -e 's|@am_log_wflags@|--am-chk|' \
    -e 's|@log_wflags@|--chk|' \
    < wrapper.skel > chk-wrapper

sed -e 's|@am_log_wflags@|-am-test|' \
    -e 's|@log_wflags@|-test|' \
    < wrapper.skel > test-wrapper

sed -e 's|@am_log_wflags@|am|' \
    -e 's|@log_wflags@|_|' \
    < wrapper.skel > bin/noext-wrapper

# 'test-wrapper' is deliberately not made executable.
chmod a+x chk-wrapper bin/noext-wrapper

# Not needed anymore.
rm -f wrapper.skel

cat > 1.chk << 'END'
#! /bin/sh
exit 0
END
chmod a+x 1.chk
cp 1.chk 2.test
cp 1.chk 3
cp 1.chk 4.c.chk
cp 1.chk 5.suf
cp 1.chk sub/test

$ACLOCAL
$AUTOCONF
$AUTOMAKE

./configure
$MAKE
VERBOSE=yes $MAKE check
ls -l . sub

test ! -e BAD.log
test ! -e BAD.trs

echo 'chk-wrapper 1.chk --am-chk --chk' > 1.exp
echo 'test-wrapper 2.test -am-test -test' > 2.exp
echo 'noext-wrapper 3 am _' > 3.exp
echo 'chk-wrapper 4.c.chk --am-chk --chk' > 4.c.exp
echo 'noext-wrapper 5.suf am _' > 5.suf.exp
echo 'noext-wrapper sub/test am _' > sub/test.exp

st=0
for x in 1 2 3 4.c 5.suf sub/test; do
  cat $x.log
  diff $x.exp $x.log || st=1
done

exit $st
