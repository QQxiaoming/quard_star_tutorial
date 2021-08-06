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
#  - which global test result derives from different test results
#    mixed in a single script?

. test-init.sh

. tap-setup.sh

cat > ok.test <<END
1..3
ok 1
not ok 2 # TODO
ok 3 # SKIP
END

cat > skip.test <<'END'
1..3
ok 1 # SKIP
ok 2 # SKIP
ok 3 # SKIP
END

cat > skipall.test <<'END'
1..0 # SKIP
foo
# bar
END

cat > fail.test <<'END'
1..1
not ok 1
END

(sed '1s/.*/1..4/' ok.test && echo 'not ok 4') > fail2.test

cat > xpass.test <<'END'
1..1
ok 1 # TODO
END

(sed '1s/.*/1..4/' ok.test && echo 'ok 4 # TODO') > xpass2.test

echo 'Bail out!' > bail.test

(cat ok.test && echo 'Bail out!') > bail2.test

cat > bail3.test <<'END'
1..0 # SKIP
Bail out!
END

# Too many tests.
cat > error.test <<'END'
1..2
ok 1
ok 2 # SKIP
not ok 3
not ok 4 # TODO
END

# Too few tests.
cat > error2.test <<'END'
1..4
ok 1
not ok 2 # TODO
ok 3 # SKIP
END

# Repeated plan.
cat > error3.test <<'END'
1..2
1..2
ok 1
ok 2
END

# Too many tests, after a "SKIP" plan.
cat > error4.test <<'END'
1..0 # SKIP
ok 1
ok 2
END

# Tests out of order.
cat > error5.test <<'END'
1..4
not ok 1 # TODO
ok 3
ok 2
ok 4
END

# Wrong test number.
cat > error6.test <<'END'
1..2
ok 1 # SKIP
ok 7
END

# No plan.
cat > error7.test <<'END'
ok 1 # SKIP
ok 2 # TODO
not ok 3 # TODO
ok 4
END

cat > hodgepodge.test <<'END'
1..2
not ok 1
ok 2 # TODO
Bail out!
END

cat > hodgepodge-all.test <<'END'
1..4
ok 1
ok 2 # SKIP
not ok 2 # TODO
not ok 3
ok 4 # TODO
Bail out!
END

tests=$(echo *.test) # Also required later.

run_make -O -e FAIL TESTS="$tests" check

# Dirty trick required here.
for tst in $(echo " $tests " | sed 's/\.test / /'); do
  echo :copy-in-global-log: yes >> $tst.trs
done

rm -f test-suite.log
run_make -e FAIL TESTS="$tests" test-suite.log
cat test-suite.log

have_rst_section ()
{
  eqeq=$(echo "$1" | sed 's/./=/g')
  # Assume $1 contains no RE metacharacters.
  sed -n "/^$1$/,/^$eqeq$/p" test-suite.log > got
  (echo "$1" && echo "$eqeq") > exp
  cat exp
  cat got
  diff exp got
}

have_rst_section 'PASS: ok'
have_rst_section 'SKIP: skip'
have_rst_section 'SKIP: skipall'
have_rst_section 'FAIL: fail'
have_rst_section 'FAIL: fail2'
have_rst_section 'FAIL: xpass'
have_rst_section 'FAIL: xpass2'
have_rst_section 'ERROR: bail'
have_rst_section 'ERROR: bail2'
have_rst_section 'ERROR: bail3'
have_rst_section 'ERROR: error'
have_rst_section 'ERROR: error2'
have_rst_section 'ERROR: error3'
have_rst_section 'ERROR: error4'
have_rst_section 'ERROR: error5'
have_rst_section 'ERROR: error6'
have_rst_section 'ERROR: error7'
have_rst_section 'ERROR: hodgepodge'
have_rst_section 'ERROR: hodgepodge-all'

:
