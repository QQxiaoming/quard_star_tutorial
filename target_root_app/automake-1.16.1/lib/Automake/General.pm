# Copyright (C) 2001-2018 Free Software Foundation, Inc.

# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2, or (at your option)
# any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.

package Automake::General;

use 5.006;
use strict;
use Exporter;
use File::Basename;

use vars qw (@ISA @EXPORT);

@ISA = qw (Exporter);
@EXPORT = qw (&uniq &none $me);

# Variable we share with the main package.  Be sure to have a single
# copy of them: using 'my' together with multiple inclusion of this
# package would introduce several copies.
use vars qw ($me);
$me = basename ($0);

# END
# ---
# Exit nonzero whenever closing STDOUT fails.
sub END
{
  # This is required if the code might send any output to stdout
  # E.g., even --version or --help.  So it's best to do it unconditionally.
  if (! close STDOUT)
    {
      print STDERR "$me: closing standard output: $!\n";
      $? = 74; # EX_IOERR
      return;
    }
}


# @RES
# uniq (@LIST)
# ------------
# Return LIST with no duplicates.
sub uniq (@)
{
   my @res = ();
   my %seen = ();
   foreach my $item (@_)
     {
       if (! exists $seen{$item})
	 {
	   $seen{$item} = 1;
	   push (@res, $item);
	 }
     }
   return wantarray ? @res : "@res";
}

# $RES
# none (&PRED, @LIST)
# ------------
# Return 1 when no element in LIST satisfies predicate PRED otherwise 0.
sub none (&@)
{
  my ($pred, @list) = @_;
  my $res = 1;
  foreach my $item (@list)
    {
      if ($pred->($item))
        {
          $res = 0;
          last;
        }
    }
  return $res;
}

1; # for require
