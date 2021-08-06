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

# Check test counts in the testsuite summary, with test drivers allowing
# multiple test results per test script, and for a huge number of tests.
# Incidentally, this test also checks that the testsuite summary doesn't
# give any bug-report address if it's not defined.

. test-init.sh

for s in trivial-test-driver extract-testsuite-summary.pl; do
  cp "$am_testaux_srcdir/$s" . || fatal_ "failed to fetch auxiliary script $s"
done

br='============================================================================'

header="\
${br}
Testsuite summary for $me 1.0
${br}"

footer="\
${br}
See ./test-suite.log
${br}"

echo AC_OUTPUT >> configure.ac

cat > Makefile.am << 'END'
TEST_LOG_DRIVER = $(SHELL) $(srcdir)/trivial-test-driver
TESTS = all.test
# Without this, the test driver will be horrendously slow.
END

cat > all.test <<'END'
#!/bin/sh
cat results.txt || { echo ERROR: weird; exit 99; }
END
chmod a+x all.test

$PERL -w -e '
  use warnings FATAL => "all";
  use strict;

  my $base = 1000;
  my %count = (
    TOTAL => $base * 1000,
    PASS  => $base * 700,
    SKIP  => $base * 200,
    XFAIL => $base * 80,
    FAIL  => $base * 10,
    XPASS => $base * 7,
    ERROR => $base * 3,
  );
  my @results = qw/PASS SKIP XFAIL FAIL XPASS ERROR/;

  open (RES, ">results.txt") or die "opening results.txt: $!\n";
  open (CNT, ">count.txt") or die "opening count.txt: $!\n";

  printf CNT "# %-6s %d\n", "TOTAL:", $count{TOTAL};
  for my $res (@results)
    {
      my $uc_res = uc $res;
      print STDERR "Generating list of $res ...\n";
      for (1..$count{$res})
        {
          print RES "$uc_res: $_\n";
        }
      printf CNT "# %-6s %d\n", $res . ":", $count{$res};
    }
'

(echo "$header" && cat count.txt && echo "$footer") > summary.exp

$ACLOCAL
$AUTOMAKE -a
$AUTOCONF

./configure

($MAKE check || touch make.fail) | tee stdout
test -f make.fail

$PERL extract-testsuite-summary.pl stdout > summary.got
cat summary.exp
cat summary.got
diff summary.exp summary.got || exit 1

:
