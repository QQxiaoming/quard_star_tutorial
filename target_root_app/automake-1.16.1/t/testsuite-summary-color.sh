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

# Check coloring of the testsuite summary.

. test-lib.sh

use_colors=yes; use_vpath=no
. testsuite-summary-checks.sh

./configure

# ANSI colors.
red='[0;31m'
grn='[0;32m'
lgn='[1;32m'
blu='[1;34m'
mgn='[0;35m'
brg='[1m';
std='[m';

success_header="\
${grn}${br}${std}
${grn}Testsuite summary for GNU AutoFoo 7.1${std}
${grn}${br}${std}"

success_footer=${grn}${br}${std}

failure_header="\
${red}${br}${std}
${red}Testsuite summary for GNU AutoFoo 7.1${std}
${red}${br}${std}"

failure_footer="\
${red}${br}${std}
${red}See ./test-suite.log${std}
${red}Please report to bug-automake@gnu.org${std}
${red}${br}${std}"

do_check '' <<END
$success_header
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
$success_header
${brg}# TOTAL: 1${std}
${grn}# PASS:  1${std}
# SKIP:  0
# XFAIL: 0
# FAIL:  0
# XPASS: 0
# ERROR: 0
$success_footer
END

do_check skip.t <<END
$success_header
${brg}# TOTAL: 1${std}
# PASS:  0
${blu}# SKIP:  1${std}
# XFAIL: 0
# FAIL:  0
# XPASS: 0
# ERROR: 0
$success_footer
END

do_check xfail.t <<END
$success_header
${brg}# TOTAL: 1${std}
# PASS:  0
# SKIP:  0
${lgn}# XFAIL: 1${std}
# FAIL:  0
# XPASS: 0
# ERROR: 0
$success_footer
END

do_check fail.t <<END
$failure_header
${brg}# TOTAL: 1${std}
# PASS:  0
# SKIP:  0
# XFAIL: 0
${red}# FAIL:  1${std}
# XPASS: 0
# ERROR: 0
$failure_footer
END

do_check xpass.t <<END
$failure_header
${brg}# TOTAL: 1${std}
# PASS:  0
# SKIP:  0
# XFAIL: 0
# FAIL:  0
${red}# XPASS: 1${std}
# ERROR: 0
$failure_footer
END

do_check error.t <<END
$failure_header
${brg}# TOTAL: 1${std}
# PASS:  0
# SKIP:  0
# XFAIL: 0
# FAIL:  0
# XPASS: 0
${mgn}# ERROR: 1${std}
$failure_footer
END

do_check pass.t xfail.t skip.t <<END
$success_header
${brg}# TOTAL: 3${std}
${grn}# PASS:  1${std}
${blu}# SKIP:  1${std}
${lgn}# XFAIL: 1${std}
# FAIL:  0
# XPASS: 0
# ERROR: 0
$success_footer
END

do_check pass.t fail.t skip.t <<END
$failure_header
${brg}# TOTAL: 3${std}
${grn}# PASS:  1${std}
${blu}# SKIP:  1${std}
# XFAIL: 0
${red}# FAIL:  1${std}
# XPASS: 0
# ERROR: 0
$failure_footer
END

do_check pass.t xfail.t xpass.t <<END
$failure_header
${brg}# TOTAL: 3${std}
${grn}# PASS:  1${std}
# SKIP:  0
${lgn}# XFAIL: 1${std}
# FAIL:  0
${red}# XPASS: 1${std}
# ERROR: 0
$failure_footer
END

do_check skip.t xfail.t error.t <<END
$failure_header
${brg}# TOTAL: 3${std}
# PASS:  0
${blu}# SKIP:  1${std}
${lgn}# XFAIL: 1${std}
# FAIL:  0
# XPASS: 0
${mgn}# ERROR: 1${std}
$failure_footer
END

do_check pass.t skip.t xfail.t fail.t xpass.t error.t <<END
$failure_header
${brg}# TOTAL: 6${std}
${grn}# PASS:  1${std}
${blu}# SKIP:  1${std}
${lgn}# XFAIL: 1${std}
${red}# FAIL:  1${std}
${red}# XPASS: 1${std}
${mgn}# ERROR: 1${std}
$failure_footer
END

:
