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

# TAP support:
#  - which log files get copied in the global log?

. test-init.sh

. tap-setup.sh

cat > ok.test << 'END'
1..5
ok 1
ok 2
ok 3
not seen in global log
ok 4
ok 5
END

cat > top << 'END'
1..6
ok 1
ok 2
ok 3
END

cat > bot << 'END'
ok 5
ok 6
END

cat top - bot > skip.test << 'END'
ok # SKIP
::skip::
END

cat top - bot > todo.test << 'END'
not ok # TODO
::todo::
END

cat top - bot > fail.test << 'END'
not ok
::fail::
END

cat top - bot > xpass.test << 'END'
ok # TODO
::xpass::
END

cat top - bot > bail.test << 'END'
::bail::
Bail out!
END

cat top - bot > error.test << 'END'
::error::
1..7
END

# Created with "dd if=/dev/urandom count=1 | base64-encode"
cat > hodgepodge <<'END'
1+0 records in
1+0 records out
512 bytes (512 B) copied, 0.000241092 s, 2.1 MB/s
gdivnV4VhL4DOzhE3zULJuun3PwqqQqMdATVcZbIQkNgyRvNBoqqHMBQEs7QsjDbp2nK+Szz
EcelGyvXmHrW7yImaom6Yrg95k31VCmp/pGDRnTDwDrcOPJiv9jDReEmTAQkPuqLO+mFNly+
DDHM9fNbzGumstsQ3wq3DOXz1pCV3JXwhjeaHSboeEbmr55bX0XHLSKaecctA0rXDXEyZWZ/
ODlawSrAXzw0H7a+xBwjnNXZ3zYiwk3x+WQrPqNjb+qXiLLTxAKzx2/KnaFhxkPlte5jPRNB
FciDolL+H/10LsCdSzLOrGnY2zH6vL2JMZfxjnb73zWFcdWWE01LTD7wpN5O1MP3+N47lcVe
lWbkD04cJvhwxLElYSO24B743GG5EyGYt9SeZRE6xbgwq3fVOS8KqjwGxwi4adSBTtw0CV8W
S/6n8dck1vBvjA+qpk0zMSYSqc3+jzW9UiGTmTEIwfw80p/lGNsfjcNBJ86nFkWUnHmrsi8k
Dv57sK70mTg239g08f5Uvdga/5UreMBSgB0hUj5sbq57r7B1fsVr0Kag468la8zKy3ZEZ0gX
++sbaJ9WGHhnKvjooeH+4Y6HwAFsdINde++FlCmp4EuNKKEEuXbSKLaOTy3+6pJ2DYdvRCL+
frZwxH4hcrw8qh+8IakB02viewZS/qT57v4=
END

exec 5>misc.test
echo 'ok # SKIP' >&5
cat hodgepodge >&5
echo 'not ok # TODO' >&5
echo 'ok' >&5
exec 5>&-

cat > skipall.test << 'END'
1..0 # SKIP all
END

run_make -e IGNORE TESTS="$(echo *.test)" check
cat test-suite.log

grep ':.*ok|not seen' test-suite.log && exit 1

for s in skip todo fail xpass bail error; do
  $FGREP "::$s::" test-suite.log
done

grep '^1\.\.0 # SKIP all$' test-suite.log

test_suite_contents=$(cat test-suite.log)
hodgepodge_contents=$(cat hodgepodge)
case $test_suite_contents in
  *"$hodgepodge_contents"*) ;;
  *) exit 1;;
esac

:
