#! /bin/sh
# Copyright (C) 2012-2018 Free Software Foundation, Inc.
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

# Check interactions between the parallel test harness and "make -n".
# See also sister test 'parallel-tests-dry-run-2.sh'.

. test-init.sh

echo AC_OUTPUT >> configure.ac

cat > Makefile.am <<'END'
TESTS = foo.test bar.test
$(TESTS):
END

$ACLOCAL
$AUTOCONF
$AUTOMAKE -a

./configure

# Avoid confusion with test logs.
rm -f config.log

log_files='test-suite.log foo.log bar.log'

all_exist ()
{
  st=0
  for i in $*; do
    test -f $i || { echo File $i not found; st=1; }
  done
  test $st -eq 0
}

none_exist ()
{
  st=0
  for i in $*; do
    if test -r $i || test -f $i; then
      echo File $i found
      st=1
    fi
  done
  test $st -eq 0
}

for targ in check recheck $log_files; do
  $MAKE -n "$targ"
  none_exist $log_files
done

touch $log_files

$MAKE -n mostlyclean
all_exist $log_files
$MAKE -n clean
all_exist $log_files

cat > foo.test <<'END'
#! /bin/sh
exit 0
END

cat > bar.test <<'END'
#! /bin/sh
exit 1
END

chmod a+x foo.test bar.test

$MAKE check && exit 1

for targ in recheck clean mostlyclean distclean; do
  $MAKE -n "$targ"
  all_exist $log_files
done

:
