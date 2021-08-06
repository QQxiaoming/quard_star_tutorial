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

# Check test counts in the testsuite summary.

. test-lib.sh

use_colors=no; use_vpath=no
. testsuite-summary-checks.sh

./configure

header="\
${br}
Testsuite summary for GNU AutoFoo 7.1
${br}"

success_footer=${br}

failure_footer="\
${br}
See ./test-suite.log
Please report to bug-automake@gnu.org
${br}"

# Corner cases.

do_check '' <<END
$header
# TOTAL: 0
# PASS:  0
# SKIP:  0
# XFAIL: 0
# FAIL:  0
# XPASS: 0
# ERROR: 0
$success_footer
END

do_check pass.t <<END
$header
# TOTAL: 1
# PASS:  1
# SKIP:  0
# XFAIL: 0
# FAIL:  0
# XPASS: 0
# ERROR: 0
$success_footer
END

do_check fail.t <<END
$header
# TOTAL: 1
# PASS:  0
# SKIP:  0
# XFAIL: 0
# FAIL:  1
# XPASS: 0
# ERROR: 0
$failure_footer
END

# Some simpler checks, with low or moderate number of tests.

do_check skip.t skip2.t skip3.t xfail.t xfail2.t <<END
$header
# TOTAL: 5
# PASS:  0
# SKIP:  3
# XFAIL: 2
# FAIL:  0
# XPASS: 0
# ERROR: 0
$success_footer
END

do_check pass.t pass2.t xfail.t xpass.t error.t error2.t <<END
$header
# TOTAL: 6
# PASS:  2
# SKIP:  0
# XFAIL: 1
# FAIL:  0
# XPASS: 1
# ERROR: 2
$failure_footer
END

pass_count=22
skip_count=19
xfail_count=21
fail_count=18
xpass_count=23
error_count=17
tests_count=120

 pass=$(seq_ 1 $pass_count  | sed 's/.*/pass-&.t/')
 skip=$(seq_ 1 $skip_count  | sed 's/.*/skip-&.t/')
xfail=$(seq_ 1 $xfail_count | sed 's/.*/xfail-&.t/')
 fail=$(seq_ 1 $fail_count  | sed 's/.*/fail-&.t/')
xpass=$(seq_ 1 $xpass_count | sed 's/.*/xpass-&.t/')
error=$(seq_ 1 $error_count | sed 's/.*/error-&.t/')

do_check $pass $skip $xfail $fail $xpass $error <<END
$header
# TOTAL: $tests_count
# PASS:  $pass_count
# SKIP:  $skip_count
# XFAIL: $xfail_count
# FAIL:  $fail_count
# XPASS: $xpass_count
# ERROR: $error_count
$failure_footer
END

# Mild stress test with a lot of test scripts.

tests_count=1888
pass_count=1403
skip_count=292
xfail_count=41
fail_count=126
xpass_count=17
error_count=9

 pass=$(seq_ 1 $pass_count  | sed 's/.*/pass-&.t/')
 skip=$(seq_ 1 $skip_count  | sed 's/.*/skip-&.t/')
xfail=$(seq_ 1 $xfail_count | sed 's/.*/xfail-&.t/')
 fail=$(seq_ 1 $fail_count  | sed 's/.*/fail-&.t/')
xpass=$(seq_ 1 $xpass_count | sed 's/.*/xpass-&.t/')
error=$(seq_ 1 $error_count | sed 's/.*/error-&.t/')

do_check $pass $skip $xfail $fail $xpass $error <<END
$header
# TOTAL: $tests_count
# PASS:  $pass_count
# SKIP:  $skip_count
# XFAIL: $xfail_count
# FAIL:  $fail_count
# XPASS: $xpass_count
# ERROR: $error_count
$failure_footer
END

:
