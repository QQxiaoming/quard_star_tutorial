# Copyright (C) 2002-2018 Free Software Foundation, Inc.
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

use Automake::Version;

my $failed = 0;

sub test_version_compare
{
  my ($left, $right, $result) = @_;
  my @leftver = Automake::Version::split ($left);
  my @rightver = Automake::Version::split ($right);
  if ($#leftver == -1)
  {
    print "can't grok \"$left\"\n";
    $failed = 1;
    return;
  }
  if ($#rightver == -1)
  {
    print "can't grok \"$right\"\n";
    $failed = 1;
    return;
  }
  my $res = Automake::Version::compare (@leftver, @rightver);
  if ($res != $result)
  {
    print "compare (\"$left\", \"$right\") = $res! (not $result?)\n";
    $failed = 1;
  }

  my $check_expected = ($result == 0 || $result == 1) ? 0 : 1;
  # Exception for 'foo' fork.
  $check_expected = 1
    if ($right =~ /foo/ && !($left =~ /foo/));

  my $check = Automake::Version::check ($left, $right);
  if ($check != $check_expected)
  {
    print "check (\"$left\", \"$right\") = $check! (not $check_expected?)\n";
    $failed = 1;
  }
}

sub test_bad_versions
{
  my ($ver) = @_;
  my @version = Automake::Version::split ($ver);
  if ($#version != -1)
  {
    print "shouldn't grok \"$ver\"\n";
    $failed = 1;
  }
}

my @tests = (
# basics
  ['1.0', '2.0', -1],
  ['2.0', '1.0', 1],
  ['1.2', '1.2', 0],
  ['1.1', '1.2', -1],
  ['1.2', '1.1', 1],
# alphas
  ['1.4', '1.4g', -1],
  ['1.4g', '1.5', -1],
  ['1.4g', '1.4', 1],
  ['1.5', '1.4g', 1],
  ['1.4a', '1.4g', -1],
  ['1.5a', '1.3g', 1],
  ['1.6a', '1.6a', 0],
# micros
  ['1.5.1', '1.5', 1],
  ['1.5.0', '1.5', 0],
  ['1.5.4', '1.6.1', -1],
# micros and alphas
  ['1.5a', '1.5.1', 1],
  ['1.5a', '1.5.1a', 1],
  ['1.5a', '1.5.1f', 1],
  ['1.5', '1.5.1a', -1],
  ['1.5.1a', '1.5.1f', -1],
  ['1.5.1f', '1.5.1a', 1],
  ['1.5.1f', '1.5.1f', 0],
# special exceptions
  ['1.6-p5a', '1.6.5a', 0],
  ['1.6', '1.6-p5a', -1],
  ['1.6-p4b', '1.6-p5a', -1],
  ['1.6-p4b', '1.6-foo', 1],
  ['1.6-p4b', '1.6a-foo', -1],
  ['1.6-p5', '1.6.5', 0],
  ['1.6a-foo', '1.6a-foo', 0],
);

my @bad_versions = (
  '', 'a', '1', '1a', '1.2.3.4', '-1.2'
);

test_version_compare (@{$_}) foreach @tests;
test_bad_versions ($_) foreach @bad_versions;

exit $failed;
