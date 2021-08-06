# Copyright (C) 2003-2018 Free Software Foundation, Inc.
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

use Automake::Wrap qw/wrap makefile_wrap/;

my $failed = 0;

sub test_wrap
{
  my ($in, $exp_out) = @_;

  my $out = &wrap (@$in);
  if ($out ne $exp_out)
    {
      print STDERR "For: @$in\nGot:\n$out\nInstead of:\n$exp_out\n---\n";
      ++$failed;
    }
}

sub test_makefile_wrap
{
  my ($in, $exp_out) = @_;

  my $out = &makefile_wrap (@$in);
  if ($out ne $exp_out)
    {
      print STDERR "For: @$in\nGot:\n$out\nInstead of:\n$exp_out\n---\n";
      ++$failed;
    }
}

my @tests = (
  [["HEAD:", "NEXT:", "CONT", 13, "v" ,"a", "l", "ue", "s", "values"],
"HEAD:v aCONT
NEXT:l ueCONT
NEXT:sCONT
NEXT:values
"],
  [["rule: ", "\t", " \\", 20, "dep1" ,"dep2", "dep3", "dep4", "dep5",
    "dep06", "dep07", "dep08"],
"rule: dep1 dep2 \\
\tdep3 dep4 \\
\tdep5 dep06 \\
\tdep07 \\
\tdep08
"],
  [["big header:", "big continuation:", " END", 5, "diag1", "diag2", "diag3"],
"big header:diag1 END
big continuation:diag2 END
big continuation:diag3
"],
  [["big header:", "cont: ", " END", 16, "word1", "word2"],
"big header: END
cont: word1 END
cont: word2
"],
  [["big header:", "", " END", 16, "w1", "w2 ", "w3"],
"big header: END
w1 w2 w3
"]);

my @makefile_tests = (
  [["target:"],
"target:
"],
  [["target:", "\t"],
"target:
"],
  [["target:", "\t", "prereq1", "prereq2"],
"target: prereq1 prereq2
"],
  [["target: ", "\t", "this is a long list of prerequisites ending in space",
    "so that there is no need for another space before the backslash",
    "unlike in the second line"],
"target: this is a long list of prerequisites ending in space \\
\tso that there is no need for another space before the backslash \\
\tunlike in the second line
"]);

test_wrap (@{$_}) foreach @tests;
test_makefile_wrap (@{$_}) foreach @makefile_tests;

exit $failed;
