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

package Automake::Language;

use 5.006;
use strict;

use Class::Struct ();
Class::Struct::struct (
	# Short name of the language (c, f77...).
	'name' => "\$",
	# Nice name of the language (C, Fortran 77...).
	'Name' => "\$",

	# List of configure variables which must be defined.
	'config_vars' => '@',

	# 'pure' is '1' or ''.  A 'pure' language is one where, if
	# all the files in a directory are of that language, then we
	# do not require the C compiler or any code to call it.
	'pure'   => "\$",

	'autodep' => "\$",

	# Name of the compiling variable (COMPILE).
	'compiler'  => "\$",
	# Content of the compiling variable.
	'compile'  => "\$",
	# Flag to require compilation without linking (-c).
	'compile_flag' => "\$",
	'extensions' => '@',
	# A subroutine to compute a list of possible extensions of
	# the product given the input extensions.
	# (defaults to a subroutine which returns ('.$(OBJEXT)', '.lo'))
	'output_extensions' => "\$",
	# A list of flag variables used in 'compile'.
	# (defaults to [])
	'flags' => "@",

	# Any tag to pass to libtool while compiling.
	'libtool_tag' => "\$",

	# The file to use when generating rules for this language.
	# The default is 'depend2'.
	'rule_file' => "\$",

	# Name of the linking variable (LINK).
	'linker' => "\$",
	# Content of the linking variable.
	'link' => "\$",

	# Name of the compiler variable (CC).
	'ccer' => "\$",

	# Name of the linker variable (LD).
	'lder' => "\$",
	# Content of the linker variable ($(CC)).
	'ld' => "\$",

	# Flag to specify the output file (-o).
	'output_flag' => "\$",
	'_finish' => "\$",

	# This is a subroutine which is called whenever we finally
	# determine the context in which a source file will be
	# compiled.
	'_target_hook' => "\$",

	# If TRUE, nodist_ sources will be compiled using specific rules
	# (i.e. not inference rules).  The default is FALSE.
	'nodist_specific' => "\$");


sub finish ($)
{
  my ($self) = @_;
  if (defined $self->_finish)
    {
      &{$self->_finish} (@_);
    }
}

sub target_hook ($$$$%)
{
    my ($self) = @_;
    if (defined $self->_target_hook)
    {
	$self->_target_hook->(@_);
    }
}

1;
