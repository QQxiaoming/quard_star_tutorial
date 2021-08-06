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

# Sanity check for the automake testsuite.
# Test subroutines to report warnings, and to signal failures, skips
# and hard errors.

unset stderr_fileno_

am_create_testdir=empty
. test-init.sh

set +e

exec 5>&1

(warn_ foobar) 2>&1 1>&5 | grep '^foobar$'             || exit 1
(fail_ foo); test $? -eq 1                             || exit 1
(fail_ foo) 2>&1 1>&5 | grep "^$me: failed test: foo"  || exit 1
(skip_ foo); test $? -eq 77                            || exit 1
(skip_ foo) 2>&1 1>&5 | grep "^$me: skipped test: foo" || exit 1
(fatal_ foo); test $? -eq 99                           || exit 1
(fatal_ foo) 2>&1 1>&5 | grep "^$me: hard error: foo"  || exit 1
(framework_failure_ foo); test $? -eq 99               || exit 1
(framework_failure_ foo) 2>&1 1>&5 \
  | grep "^$me: set-up failure: foo"                   || exit 1

stderr_fileno_=6

(warn_ foobar) 6>&1 1>&5 | grep '^foobar$'             || exit 1
(fail_ foo); test $? -eq 1                             || exit 1
(fail_ foo) 6>&1 1>&5 | grep "^$me: failed test: foo"  || exit 1
(skip_ foo); test $? -eq 77                            || exit 1
(skip_ foo) 6>&1 1>&5 | grep "^$me: skipped test: foo" || exit 1
(fatal_ foo); test $? -eq 99                           || exit 1
(fatal_ foo) 6>&1 1>&5 | grep "^$me: hard error: foo"  || exit 1
(framework_failure_ foo); test $? -eq 99               || exit 1
(framework_failure_ foo) 6>&1 1>&5 \
  | grep "^$me: set-up failure: foo"                   || exit 1

:
