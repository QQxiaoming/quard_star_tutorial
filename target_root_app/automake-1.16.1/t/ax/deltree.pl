#!/usr/bin/env perl
# deltree: recursively removes file and directory,
# trying to handle permissions and other complications.

# Copyright (C) 2013-2018 Free Software Foundation, Inc.

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

use strict;
use warnings FATAL => 'all';
use File::Path qw/rmtree/;

my $exit_status = 0;
local $SIG{__WARN__} = sub { warn "@_"; $exit_status = 1; };

foreach my $path (@ARGV) {
  local $@ = undef;
  rmtree ($path);
}

exit $exit_status;

# vim: ft=perl ts=4 sw=4 et
